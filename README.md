# Prim

## Description

This is a CPU based ray-tracing renderer using multi-threading capacity of CPU for faster rendering. It is not designed for performances as it is a pet project. Its goal is to reimplement diverse features related to ray-tracing and more generally rendering in order to better understand them. It is designed to be easy to understand and read, that's why I am using object oriented design for easier clarity, while still using some data oriented design when required.

The BVH implementation is totally custom and based on kd tree algorithm. It could be faster using Embree or such API.

## Features
- [x] Custom BVH mesh intersection
- [x] Scalable materials
- [x] hellos
- [ ] Embree or such BVH intersection

Check this :
https://casual-effects.com/g3d/data10/index.html#