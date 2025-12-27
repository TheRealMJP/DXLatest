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

* Does this work right in terms of hiding + intellisense with implicit casting to base?
    * Seems to work ok in terms of making methods inaccessible, however the private methods still show up in intellisense but with a lock icon

Auto-Generation vs. Hand Writing
================================

