# The SOLAr-sim output

The output file produced by `SOLAr-sim` is a ROOT file containing a `TTree` with
the simulated events, a second `TTree` describing the generators settings, and
several objects describing the configuration of the simulation at runtime.
These include the geometry, the material definitions, and the configuration of the
anode and of the photon detection system (PDS).

## The Event Tree

The Event tree (`EventTree`) is structured in different branches, 
describing the simulated events as recorded by each given subsystem. 
The three basic branches are: 
- `MCTruth`: contains the information about each particle defining the event
  initial state. Each primary particle also carries the information about the 
  secondary particles and stores all trajectories associated with the primary.
- `MCEventAnode`: contains the information about the hits recorded by the anode
  including both the ionization electrons collected by the charge pads and 
  the optical photons detected by the SiPMs. 
- `MCEventPDS`: contains the information about the hits recorded by the 
  photon detection system (PDS). The PDS is a dedicated subsystem that 
  records the scintillation light produced in the LArTPC. The PDS can be 
  configured to use different readout technologies, such as SiPMs or PMTs.

### Primary info

The `MCTruth` branch contains the information about the propagation of the 
primary particles in the experimental setup. 
The `SLArMCTruth` object contained in the branch is a collection of 
`SLArMCPrimaryInfo` objects, one for each primary particle. 
The `SLArMCPrimaryInfo` object contains the information about the primary
particle, such as its track id, its type, energy, and production vertex, time and momentum.
In addition to its own trajectory, each "primary" carries 
the trajectories of all secondary particles associated with the primary track.
The points of each trajectory are defined by their spatial coordinates and by 
the energy deposited in the step and by other attributes such as the energy 
deposited in the step, the number of scintillation photons and ionization electrons
produced, and whether the step was inside the LAr volume or not. 
The reference frame for the coordinates is defined by the center of the LAr volume
(e.g., if the setup is made by two TPCs sharing a common cathode plane, the 
origin of the reference frame is in the middle of the cathode plane).

The "hierarchical" structure of the primary information object is represented 
in the figure below (Fig. 1)

| ![SLArMCPrimaryInfo diagram](./docs/figures/umls/SLAr-MCTruth.png)           |
| :--:                                                                         |
| **Fig. 1** UML digram showing the hierarchical structure of the primary information data members |


### Photon Detection System event collection

The photon hits recorded by the "conventional" PhotoDetection System (PDS), 
i.e., not considering the hits collected by the photosensitive anode, are
stored in the `EventPDS` branch of the event tree. 
The branch contains a `SLArListEventPDS` object, which is a collection of
`SLArEventSuperCellArray` objects. Each `SLArEventSuperCellArray` 
corresponds to a "wall" of the PDS and includes a collection of
`SLArEventSuperCell` objects which recorded at least one hit. 
`SLArEventSuperCell` implements a `SLArEventHitsCollection` capable of 
recording photon hits in a supercell. 

The concept of a `SLArEventHitsCollection` is based on the idea of 
limiting the disk space usage in the simulation output by sacrificing precision 
beyond the needs of the simulation. Hits are stored in a map having as a key
a timestamp and as value the number of hits recorded at that time. 
The timestamp is obtained by discretizing the time of the hit with a given "clock" 
unit that is configurable with a macro command. 

Figure 2 schematically represents the structure of the event as seen by the PDS.

| ![PDS event diagram](./docs/figures/umls/SLAr-EventPDS.png) |
| :--:                                                  |
| **Fig. 2** Schematic representation of the data members making the PDS event. |


### Anode event collection

Like the PDS, the anode event collection is stored in a dedicated branch
of the event tree. The branch is called `EventAnode` and contains a
`SLArListEventAnode` object, which in turn contains a collection representing 
all the anode planes that are used in the simulation. 

Each anode plane is divided into a number of "megatiles", which groups 
together several readout tiles and are represented by the
`SLArEventMegatile` and `SLArEventTile` objects respectively. 
Accessing the tile object, one can immediately access the photon hits
recorded by the photosensors in the same way as for the PDS. 
In addition to the optical hits, the tile also contains the information about the
ionization electrons collected by the charge pads. Similarly to optical 
detector modules, each charge pad is represented by an implementation 
of `SLArEventHitsCollection`, which stores the hits in a map having as a key
the timestamp and as value the number of ionization electrons recorded at that time, 
thus approximating the induced current on the pad. 
Each pad that collected a number of electrons above a given threshold is
stored into a dedicated `std::map` in the `SLArEventTile` object, having as a key
the pad id. 

A sketch of the anode event structure is shown in Fig. 3.

| ![Anode event diagram](./docs/figures/umls/SLAr-EventAnode.png) |
| :--:                                                  |
| **Fig. 3** Schematic representation of the data members making the anode event. |


## SOLAr Event Dictionaries

To be able to access the event information in an interactive ROOT session, 
one should load the shared libraries defining the event and configuration 
objects. 
These libraries are built when compiling `SOLAr-sim` and are installed in 
the `G4SOLAR_INSTALL_DIR/lib` folder. During the installation process, 
a `rootlogon.C` file loading the libraries is created in 
`G4SOLAR_BASE_DIR/SOLArAnalysis`. In the same folder, one can
find the script `test_output.C`, which can serve as an example for accessing
simulated MC event. 


