# SOLAr-sim primary generators

`SOLAr-sim` offers several, specialized primary generators that can be composed
together to create composite initial states in the simulation. 

Generators are composed and configured in a `json` file like this:

```json
{
    "generator": [
    {
        "type" : "particlegun", 
        "label" : "my_gen", 
        "config" : { ... }
    },
    {
        "type" : "marley",
        "label" : "my_gen2",
        "config" : { ... }
    },
    ...
    ]
}
```

where the field `type` indicates the specialized generator to be used and 
`label` is a unique identifier for the generator. The `config` field is a
dictionary of parameters, many of which are specific to the generator type.

## General structure

Each specialized generator implements a common abstract base class 
`gen::SLArBaseGenerator` that provides a common interface for all generators. 
The base class provides multiple abstract methods and contains dedicated generator
for production of vertices and for the direction of the primary particles.

| ![SOLAr-sim generators diagram](./docs/figures/umls/SLAr-AllGenerators.png)  |
| :--:                                                                         |
| **Fig. 1** UML digram showing the structure of specialized primary generators. |


## Vertex generators

Vertex generators are responsible for producing the initial position and time
of the primary particles. Currently, `SOLAr-sim` supports four different types
of vertex generators: `Fixed`, `BoxSurface`, `Bulk`, and "GPS-style" generators.

All these are configured within the `config` field in the description of the 
generator with the `vertex_gen` description. The configuration of the vertex 
generators are briefly discussed in the sections below. 


### Time

The vertex generators are also responsible for setting the time of the production 
of the primary particles. 

The `gen::time::SLArTimeGenerator` time generator is configured with the mandatory `mode` field. 
Available modes are:
- `fixed` for a fixed time value
- `window` for a time window

The `event_time` field is used for the `fixed` mode, and it must
be a JSON object with the fields `val` and `unit`, where `val` is
the time value and `unit` is the unit of the time value (default is ns).

```json
"time" : {"mode" : "fixed", "event_time" : {"val" : 4.0, "unit" : "us"}}
```

The "event_window_limits" field is used for the `window` mode, and it
must be a JSON object with two subfields: `t0` and `t1`, which define
the start and end of the time window. `t0` and `t1` are JSON
objects with the fields `val` and `unit`, where `val` is the time value
and `unit` is the unit of the time value (default is ns).

```json
"time" : 
{
    "mode" : "window", 
    "event_window_limits" : 
    {
        "t0" : {"val": -10, "unit" : "ns"}, "t1" : {"val" : 10, "unit" : "ns"}
    }
}
```

### Point

The `gen::vertex::SLArPointVertexGenerator` class implements a point vertex generator. 
The vertex is set to a fixed position in the 3D space. The position can be configured
using the `Config` method, which takes a JSON object as input.

The JSON object should contain the following fields:
- `xyz`: the position of the vertex, given as a JSON object with
`val` field containing an array of three values (x, y, z) and
an optional `unit` field for the unit of the values.
- `volume` (optional): the name of the reference volume for the vertex position.

Here an example of the configuration of a point vertex generator producing 
a vertex at 15 cm on the y-axis from the center of a volume called `TPC10`
```json
"vertex_gen" : {
    "type" : "point", 
    "config" : {
        "volume" : "TPC10",
        "xyz" : {"val" : [0, 15, 0], "unit" : "cm"}
    }
}
```

### BoxSurface

The `gen::vertex::SLArBoxSurfaceVertexGenerator` generates vertexes on the surface of a box volume 
that defines the shape of a physical volume. 

The JSON configuration should contain the following fields:
- `volume`: the name of the physical volume on which surface the vertexes are generated
- `origin_face` (optional): the face of the box where the vertexes are generated. The face is specified
  using the `geo::EBoxFace` enum, which uses the following convention: 
  {kXplus=0, kXminus=1, kYplus=2, kYminus=3, kZplus=4, kZminus=5}.
  If no face is specified, the generator will use the entire box.

### Bulk
The `gen::vertex::SLArBulkVertexGenerator` class generates vertexes in a bulk volume as defined by a `G4VPhysicalVolume`.

 The generator JSON configuration should contain the following fields:
 - `volume`: the name of the physical volume where the vertexes are generated
 - `fiducial_fraction` (optional): the fraction of the volume that is used for vertex generation. 
   (only works for box-shape volumes). Setting this value to 1.0 means that the whole 
   volume is used. If smaller than 1, the volume is shrunk by the same amount 
   in all directions until reaching the requested volume fraction.
 - `avoid_daughters` (optional): if set to true, the generator will avoid generating vertexes
   inside daughter volumes of the specified volume.
 - `material` (optional): the material of the volume. If set, the generator will only generate vertexes
   where the material matches the specified one.

### GPS-style vertex
The `gen::vertex::SLArGPSVertexGenerator` class is a wrapper around `G4SPSPosDistribution`,
which is used to generate random vertex positions with the Geant4 General Particle 
Source (GPS) interface ([link](https://geant4-userdoc.web.cern.ch/UsersGuides/ForApplicationDeveloper/html/GettingStarted/generalParticleSource.html)). 
It provides methods to configure the vertex generator, shoot vertices, and export the configuration in JSON format.

The JSON configuration fields are supposed to be in the same format used in GPS macro commands
and are: 
- `type`: the type of the source (`Point`, `Beam`, `Plane`, `Surface`, `Volume`)
- `shape`: only for `Surface` and `Volume` source type. 
  For "Surface" type, shapes `Circle`, `Annulus`, `Ellipse`, `Square`, `Rectangle` are available.
  For both `Surface` and `Volume` types, the shape can be set to 
  `Sphere`, `Ellipsoid`, `Cylinder`, `Para`
- `center`: the center of the source, given as array of three values with key "val". 
  The unit of the values can be set with the "unit" key (default is mm).
- `reference_volume` (optional): the name of the reference volume for the source placement.
- `halfx`, `halfy`, `radius`, `inner_radius`: the dimensions of the source, 
  depending on the type and shape. Values must be given in JSON object with key "val", 
  and optionally with key "unit" for the unit.
- `beam_sigma_r`, `beam_sigma_x`, `beam_sigma_y`: the beam size parameters 
  for the `Beam` source type. Values must be given in JSON object with
  key "val", and optionally with key "unit" for the unit.
- `rot1`, `rot2`: the rotation vectors for the source, given as arrays of three values.
- `alpha`: Used with a Parallelepiped. The angle [default 0 rad] α formed by 
  the y-axis and the plane joining the centre of the faces parallel to 
  the zx plane at y and +y.
- `theta`: Used with a Parallelepiped. Polar angle [default 0 rad] θ  
  of the line connecting the centre of the face at z to the centre of the face at +z.
- `phi`: Used with a Parallelepiped. The azimuth angle [default 0 rad] ϕ 
  of the line connecting the centre of the face at z with the centre of the face at +z. 
- `confine_to_volume`: the name of the physical volume to confine the source to.
 
## Direction generators

Similarly to the vertex generators, direction generators are responsible for producing
the initial direction of the primary particle momentum. 
The direction generators are configured in the `config` field of the generator
with the `direction` description. 

Currently, `SOLAr-sim` supports three different types of direction generators:
- `Fixed`: a fixed direction
- `Isotropic`: isotropic direction
- `GPS`: a GPS-style direction generator
- `Sun`: a generator that produces random directions taking into account the 
    nadir angle exposure to the Sun.

Note that specific types of generators override the direction generator 
settings. For instance, particles produced by `gen::SLArDecay0GeneratorAction`
are not affected by the direction generator settings, and in case must be 
controlled with the `g4_bxdecay0` custom commands. Similarly, particles with vertices
produced by `gen::vertex::SLArBoxSurfaceVertexGenerator`
by the `gen::SLArExternalGeneratorAction` are always produced isotropic, 
with the direction along the normal to the surface of the box going towards the box centre. 

### Fixed
The `gen::direction::SLArFixedDirectionGenerator` class generates a fixed direction 
for primary particles. The direction is specified in the configuration JSON object 
using the `axis` field, which can be a three-dimensional array or an object with a "val" field
set to a three-dimensional array. 
One can also specify a reference physical volume using the `volume` field.

### Isotropic
The `gen::direction::SLArIsotropicDirectionGenerator` class generates isotropic 
directions for primary particles. The direction is sampled uniformly on a unit sphere.

### GPS-style direction
The `gen::direction::SLArGPSDirectionGenerator` class is a wrapper around `G4SPSAngDistribution`, 
which is used to generate the direction of primary particles with the Geant4 
General Particle Source ([link](https://geant4-userdoc.web.cern.ch/UsersGuides/ForApplicationDeveloper/html/GettingStarted/generalParticleSource.html)).

The configuration is done using a JSON object which is supposed 
to replicate the same direction settings defined in the GPS macro commands:
- `type`: the type of the source 
  (`iso` [default], `cos`, `planar`, `beam1d`, `beam2d`, `focused`, `user`)
- `rot1`, `rot2`: Defines the rotation vectors AR1 [default (1,0,0)] and 
  AR2 [default (0,1,0)] for the angular distribution and are not necessarily 
  unit vectors.  
- `mintheta`, `maxtheta`: the minimum and maximum theta angles (theta = 0 -> -z direction). 
  Values must be given in JSON object with key "val", and optionally with key 
  "unit" for the unit.
- `minphi`, `maxphi`: the minimum and maximum phi angles. Values must be given in
  JSON object with key "val", and optionally with key "unit" for the unit.
- `sigma_x`, `sigma_y`, `sigma_r`: Sets the standard deviation [default 0 rad] 
  of beam directional profile in the radial, x- and y- direction. 
- `focus_point`: the focus point for a beam source. Its coordinates are given
  in a JSON object with key "val" associated to a three-dimensional array,
  and optionally with key "unit" for the unit.
- `theta_hist`, `phi_hist`: the name of the histogram file and object to be used
  for the angular distribution. The histogram must be a TH1D object with 
  angles in radians. The configuration is done using a JSON object with the
  `type` field set to "user", and the `filename` and `objname` fields.

## Specialized generators

### Particle Gun

### Particle Bomb

### MARLEY

### Decay0

### CRY

### RadSource

### GENIE
