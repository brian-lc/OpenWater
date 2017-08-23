# OpenWater
This is an open source hardware project. It is published under the CERN OpenHardware License. https://www.ohwr.org/documents/294

#Note!
This is very much a work in progress. All designs are subject to change.

## Goal
To create and share an open source water monitoring system that was capable of measuring water temperature, pH, and ORP. 

This repo contains all the hardware, software, 3D models, and instructions on how to create your own version. 

## Bill of Materials (all versions)
https://docs.google.com/spreadsheets/d/1hH_PX7qE9Eqqa5zU-xhwlhM-gbqpwk3e61Kn0q8U03g/edit?usp=sharing


# History

## V3 - Pelican case - WIP
This version rethinks the whole approach and tries to again use a simple enclosure to house the electronics and keep things water-tight. Forget submerged housings and dealing with buoyancy. This will use some foam and the Pelican case to keep the device on top of the water. It'll be bigger but it will also allow the placement of a solar panel on top. 


## V2 - Inflatable 
This design throws away the idea that the enclosure needs to be rigid. Using a flexible tubing (originally designed for industrial processing) I created an enclosure that could be inflated/deflated to adjust for buoyancy. What I learned was that 1) buoyancy is hard to get right when it's a DIY project. Even the slightest off center mass (like a cable on one side) could cause the whole setup to list. Also, USB cables rust. Also also, making something water tight is hard. The first time I floated this in an over-night test the whole inside filled with water because the air pressure stem didn't have enough back-pressure to keep the water out. Luckily none of the electronics were harmed. 

![V2 - Inflatable Prototype](/guide/images/v2_bucket_test.jpg?raw=true "V2 - Inflatable Prototype bucket test")

## V1 - Blender Bottle
My first attempt at a cheap ($11 USD) enclosure using an off-the-shelf blender bottle. With just a few components this design came together well but it proved to be too difficult to service the battery. Also, the cabling situation was a nightmare. The probes each come with about 5 feet of coax. Dealing with it is a real pain. 

![V1 - BlenderBottle](/guide/images/v1_assembled.jpg?raw=true "V1 - BlenderBottle assembled")


### Resources and other projects
https://www.voltaicsystems.com/blog/building-solar-powered-cloud-connected-devices/

