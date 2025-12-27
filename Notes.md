Goals
=============

### Primary Goals

* Hide "deprecated" functionality, only expose the latest

### Secondary Goals

* Add inline documentation through comments
* Combine interface revisions into a single function list
* Don't show deprecated functions in intellisense

### Tertiary Goals

* Reasonable helper/extension functions where it makes sense
* Seamless interop of any new types being passed to native D3D12 functions
* Seamless interop of any native D3D12 types being passed to DXL functions
* Allow directly replacing native D3D12 interface pointers or structs with DXL types with no code changes
* Allow directly replacing DXL types with native D3D12 interface pointers or structs with no code changes
    * Any extension/helper function could prevent this, need to be clear on extensions
* Add reasonable default values for structs
    * Requires redefining structs with our own types
* Inline comments for structs
    * Also requires redefining structs

Possible Design Directions
==========================

Auto-Generation vs. Hand Writing
================================