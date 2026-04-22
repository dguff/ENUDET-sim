#!/usr/bin/env python3

###############################################################################
# jcomposer: A JSON composition tool with support for includes, jq filters, 
#            and schema validation.
# author: Daniele Guffanti <daniele.guffanti_at_mib_dot_infn_dot_it>
# Usage:
#   jcomposer.py [options] <template.json>
# Options:
#   -o, --output <file>    Output file (default: stdout)
#   --debug                Enable debug mode
#   --schema <schema.json> Validate output against JSON schema
###############################################################################

import os
import json
import subprocess
import argparse
from pathlib import Path

SSIM_PREFIX_PATH = os.environ.get("SSIM_PREFIX_PATH", ".")
p = Path(SSIM_PREFIX_PATH)

CONTEXT_PATHS = {}
if p.exists() and p.is_dir():
    CONTEXT_PATHS = {
            "geometry": [
                SSIM_PREFIX_PATH+"/assets/geometry"
                ],
            "generator": [
                SSIM_PREFIX_PATH+"/assets/macros"
                ],
            "default": [ "." ]
            }
else:
    CONTEXT_PATHS = {
        "geometry": [ "." ],
        "generator": [ "." ],
        "default": [ "." ]
        }

########################################
# Utilities
########################################

def run_jq_filter(data, filt):
    if not filt:
        return data
    proc = subprocess.run(
        ["jq", filt],
        input=json.dumps(data),
        text=True,
        capture_output=True
    )
    if proc.returncode != 0:
        raise RuntimeError(f"jq filter error:\n{proc.stderr}")
    return json.loads(proc.stdout)

def deep_merge(a, b):
    if isinstance(a, dict) and isinstance(b, dict):
        result = dict(a)
        for k, v in b.items():
            if k in result:
                result[k] = deep_merge(result[k], v)
            else:
                result[k] = v
        return result
    return b

########################################
# Include parsing
########################################

def parse_include(s):
    mode = None

    if s.endswith("]") and "[" in s:
        idx = s.rfind("[")
        mode = s[idx+1:-1]
        s = s[:idx]

    if "::" in s:
        file, filt = s.split("::", 1)
    else:
        file, filt = s, None

    return file, filt, mode or "merge"

########################################
# File resolution
########################################

def build_search_paths(context, user_paths):
    paths = []

    # CLI override
    if user_paths:
        paths.extend(user_paths.split(":"))

    # context defaults
    if context in CONTEXT_PATHS:
        paths.extend(CONTEXT_PATHS[context])
    else:
        paths.extend(CONTEXT_PATHS["default"])

    # environment fallback
    env = os.environ.get("SSIM_PREFIX_PATH")
    if env:
        paths.extend(env.split(":"))

    # always include current dir
    paths.append(".")

    # remove duplicates while preserving order
    seen = set()
    result = []
    for p in paths:
        if p not in seen:
            seen.add(p)
            result.append(p)

    return result

class Resolver:
    def __init__(self, search_paths):
        self.search_paths = search_paths

    def resolve(self, fname, current_dir):
        # relative path
        if fname.startswith("./") or fname.startswith("../"):
            path = Path(current_dir) / fname
            if path.exists():
                return path.resolve()
            raise FileNotFoundError(f"Relative file not found: {fname}")

        # search paths
        for base in self.search_paths:
            for root, _, files in os.walk(base):
                if fname in files:
                    return Path(root) / fname

        raise FileNotFoundError(f"File not found: {fname}")

########################################
# Composer
########################################

class Composer:
    def __init__(self, resolver, debug=False):
        self.resolver = resolver
        self.cache = {}
        self.debug = debug

    def log(self, msg):
        if self.debug:
            print(msg)

    def load_json(self, path):
        path = str(path)
        if path in self.cache:
            self.log(f"[cache] {path}")
            return self.cache[path]

        self.log(f"[load] {path}")
        with open(path) as f:
            data = json.load(f)

        self.cache[path] = data
        return data

    def expand(self, data, current_file, stack):
        if isinstance(data, dict):
            if "@include" in data:
                inc = data["@include"]
                file, filt, mode = parse_include(inc)

                resolved = self.resolver.resolve(file, Path(current_file).parent)

                if str(resolved) in stack:
                    raise RuntimeError(
                        "Cycle detected:\n" + " -> ".join(stack + [str(resolved)])
                    )

                self.log(f"[include] {file} (filter={filt}, mode={mode})")

                base = self.load_json(resolved)
                base = self.expand(base, resolved, stack + [str(resolved)])

                # apply jq filter
                base = run_jq_filter(base, f".{filt}" if filt else ".")

                overlay = {k: v for k, v in data.items() if k != "@include"}

                if mode == "replace":
                    return base

                return deep_merge(base, {
                    k: self.expand(v, current_file, stack)
                    for k, v in overlay.items()
                })

            # normal dict
            return {
                k: self.expand(v, current_file, stack)
                for k, v in data.items()
            }

        elif isinstance(data, list):
            return [self.expand(x, current_file, stack) for x in data]

        else:
            return data

########################################
# Schema validation
########################################

def validate_schema(data, schema_path):
    try:
        import jsonschema
    except ImportError:
        raise RuntimeError("jsonschema package required for validation")

    with open(schema_path) as f:
        schema = json.load(f)

    jsonschema.validate(instance=data, schema=schema)

########################################
# CLI
########################################

def main():
    parser = argparse.ArgumentParser(description="JSON composition tool")
    parser.add_argument("template", help="Input JSON template")
    parser.add_argument("-o", "--output", help="Output file")
    parser.add_argument(
        "-c", "--context",
        default="default",
        help="Configuration context (geometry, generator, ...)"
        )
    parser.add_argument(
        "--path",
        help="Additional search paths (colon-separated)"
        )
    parser.add_argument("--debug", action="store_true", help="Enable debug mode")
    parser.add_argument("--schema", help="Validate against JSON schema")

    args = parser.parse_args()

    search_paths = build_search_paths(args.context, args.path)
    # print seach path: 
    print(f"Search paths: {search_paths}")

    resolver = Resolver(search_paths)
    composer = Composer(resolver, debug=args.debug)

    template_path = Path(args.template).resolve()

    with open(template_path) as f:
        data = json.load(f)

    result = composer.expand(data, template_path, [str(template_path)])

    if args.schema:
        validate_schema(result, args.schema)

    output = json.dumps(result, indent=2)

    if args.output:
        with open(args.output, "w") as f:
            f.write(output)
    else:
        print(output)

if __name__ == "__main__":
    main()
