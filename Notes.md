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

Possible Wrapper Design Directions
==================================

### Wrapper Class

* Store native interface pointer internally as a member
* Users must store an instance, not a pointer
* Basically a smart pointer, but without the lifetime management

+ Can implicitly decay to the native interface pointer type
+ Can implicitly convert from the native interface pointer type
+ Can be sizeof(pointer)
+ Exposes only the methods we want
+ Can add extension methods
+ Can combine methods from multiple interface versions
+ Can use our own wrappers and structs as a parameters if we want
+ Doesn't require allocation

- Not implicitly convertible in cases where arrays of pointers are passed
- Extra call overhead if not inlined
- Doesn't work with IID_PPV_ARGS, extensions (or signature changes) needed for returning wrapper types

* Can we use this in a COM smart pointer?
* Can we allow boolean operator and/or checking against nullptr without ambiguous calls?
    * Compiler won't let you hide one overload a method while exposing another, so you can't have a ToNative function that's hidden
    * Looks bool checks are fine, but you need to be consistent on const vs non-const for the operators
    * Every class having its own ToNative seems to be fine, only shows up once in intellisense
    * == and != are also ok as long as you define all of them for every class, including comparing against the wrapper

### Wrapper Class, Store as a Pointer

* Store native interface pointer internally
* Users store a pointer, not an instance

+ Native pointer type, consistent with holding onto a native interface pointer
+ Can do boolean checks and comparisons with nullptr
+ Can be sizeof(pointer)
+ Exposes only the methods we want
+ Can add extension methods
+ Can combine methods from multiple interface versions
+ Can use our own wrappers and structs as a parameters if we want

- Requires some kind of allocation or pooling somewhere to be able to return a pointer
- Can't implicitly decay to the native interface pointer type
- Can't implicitly convert from the native interface pointer type
- Extra call overhead if not inlined

### Derive From Native Interface, Expose With Using

* Derive with private inheritance and then using to add the methods you want to expose

+ Native pointer type, consistent with holding onto a native interface pointer
+ Can do boolean checks and comparisons with nullptr
+ Can implicitly decay to the native interface pointer type
+ Can be sizeof(pointer)
+ Exposes only the methods we want
+ Can add extension methods
+ Can effectively combine methods from multiple interface versions
+ Doesn't require any allocation
+ Doesn't require implementing non-extension methods
+ No call overhead

- Can't implicitly convert from the native interface pointer type
- Not implicitly convertible in cases where arrays of pointers are passed (I don't think, double check)
- Private methods still show up in intellisense but with a lock icon
- Doesn't work with IID_PPV_ARGS, extensions (or signature changes) needed for returning wrapper types

* Does this work right in terms of hiding + intellisense with implicit casting to base?
    * Seems to work ok in terms of making methods inaccessible, however the private methods still show up in intellisense but with a lock icon

### Derive From Native Interface, Expose By Reimplementing methods

* Derive with private inheritance and then re-implement methods we want to expose

+ Native pointer type, consistent with holding onto a native interface pointer
+ Can do boolean checks and comparisons with nullptr
+ Can implicitly decay to the native interface pointer type
+ Can be sizeof(pointer)
+ Exposes only the methods we want
+ Can add extension methods
+ Can effectively combine methods from multiple interface versions
+ Doesn't require any allocation

- Can't implicitly convert from the native interface pointer type
- Not implicitly convertible in cases where arrays of pointers are passed (I don't think, double check)
- Extra call overhead if not inlined
- Private methods still show up in intellisense but with a lock icon
- Doesn't work with IID_PPV_ARGS, extensions (or signature changes) needed for returning wrapper types

* Does this work right in terms of hiding + intellisense with implicit casting to base?
    * Seems to work ok in terms of making methods inaccessible, however the private methods still show up in intellisense but with a lock icon

Other Design Decisions
======================

### Redefined Structs

* Are these worth it?
+ Defaults are nice
+ Standard types are also kinda nice
+ Could maybe hide certain members, but then care needs to be taken during conversion
- Problems with passing pointers to interface methods, can't implicitly convert in that case

### Redefined Enums

* These are annoying to replicate and don't add that much value
* Probably wouldn't document or comment these all either...
* Unsure about implicit conversions with these
+ Could exclude certain options that are considered deprecated
- Annoying to replicate
- Take up lots of space

### Standard Types for Parameters

* Could be nicer than the Windows types
* But maybe it's just different for little benefit?

### Exposing Extension Methods

* On the same interface?
* Make a different interface with the extensions?
* Have a naming convention?
* Enable/disable with a macro?

What To Deprecate
=================

### Definitely

* Vertex buffers and fixed-function vertex fetch
* Bundles
* Legacy barriers
* PSO libraries
* Stream Out
* Protected Sessions
* Meta commands

### Maybes

* Descriptor tables?
    * Could be cool, not really needed that much
    * Could also remove descriptor copying maybe?
    * Need to check out root sig desc

* CPU-visible descriptor heaps
    * They're fairly pointless for SRV/UAV/CBV/Sampler heaps but the concept still needs to exist for RTV/DSV heaps
    * Also needed for ClearUnorderedAccessView

* Copying descriptors
    * Only relevant for CPU-visible descriptor heaps, pointerless for bindless
    * Maybe still useful for ClearUnorderedAccessView?

* ClearUnorderedAccessView?
    * Can't hide this if we get rid of CPU-visible heaps, which is annoying
    * Tough to provide as a helper implementation I think because you would need PSO

* Root signatures?
    * Heavy handed, but would clean up a lot of things
    * Would have to provide some sort of universal root signature, or a way to make one
    * Could be very cool/intriguing for beginners, but also risky and divergent

* Vertex shaders and index buffers?
    * Not sure this would be a good choice for beginners, mesh shaders are gnarly and have a higher startup cost because of meshlets
    * Also not sure the surface area from this is particularly high

* Geometry shaders?
    * They're not great but still have some use cases
    * Not sure the API surface area is particularly high

* Hull and Domain shaders?
    * They also have issues but mesh shaders are far from a straightforward replacement
    * Not sure the API surface area is particularly high

* MSAA
    * Not often used, but occasionally still useful
    * It has a bit of API surface area

* View instancing
    * Really not that useful
    * API surface is modest


Helper Functions and Extensions
===============================

* UTF-8 for SetName
* Create device with agility SDK and debug layer with callback
* PSO creation

Auto-Generation vs. Hand Writing
================================

* IDL (MIDL) seems not too hard make a crappy parser for, at least the D3D12 subset
    * MIDL compiler doesn't seem useful, but maybe there's a useful library somewhere? Couldn't immediately find one.
    * Would be easy to have a list of interfaces/methods to include, and results for converting parameter types if desired
    * How to include inline comments?
    * How to include extension methods?
    * Could maybe save a lot of time even on the intitial rev, or maybe could be useful for generating custom cuts with options
    * Would definitely be nice for keeping up with future additions

* Hand written is obviously full control
    * Time consuming
    * Easy to interleave comments and extensions
    * Can use macros for certain amounts of boilerplate, as well as for certain configuration options

* Maybe have a "source" header and then preprocess into an output?
    * That would be neat for making custom cuts and leaving the output clean
    * Would have to look into what preprocessor to use, options, etc.
    * Comments would need to survive
    * msvc preprocessor doesn't seem super promising for this, not a lot of options ands includes get expanded.