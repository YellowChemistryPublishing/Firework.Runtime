![Developed by atmc Badge](https://img.shields.io/badge/atmc-We%20made%20this!-%23303030?labelColor=%23c80000)
[![Build Status](https://img.shields.io/github/actions/workflow/status/YellowChemistryPublishing/Firework.Runtime/cmake.yml?branch=main&logo=github)](https://github.com/YellowChemistryPublishing/Firework.Runtime/actions/workflows/cmake.yml)

# Firework.Runtime

A a blazing fast<sup>\[citation needed\]</sup>, lightweight game and application development library that is intuitive and easy to use, whilst also being performant and efficient.

This library is _very_ in-development right now, and definitely should not be used beyond experimentation. (Some commits might not even build!)

Firework.Runtime currently comprises the following set of non-experimental libraries:

|                                   |                                                                   |
| --------------------------------- | ----------------------------------------------------------------- |
| `Firework.Runtime.GL`             | The core graphics library all rendering is done through.          |
| `Firework.Runtime.RenderPipeline` | Let's you change rendering behaviour on the fly, if you'd like.   |
| `Firework.Runtime.CoreLib`        | The fundamentals--windowing, input, system events, ECS, and more! |
| `Firework.Components.Core2D`      | Basic 2D components you might want to use.                        |

As well as the following tools and examples:

|                                  |                                                                           |
| -------------------------------- | ------------------------------------------------------------------------- |
| `example_visual_svg`             | Rendering scalable graphics from the `.svg` file.                         |
| `example_visual_text`            | See how you can draw text!                                                |
| `tool_packager`                  | The packaging utility to generate archives that can be loaded at runtime. |
| `cmake/package_archive.cmake`    | CMake include for creating a packaging target.                            |
| `cmake/precompile_shaders.cmake` | CMake include for creating a shader precompilation target.                |

## Acknowledgements

-   Truong, N., Yuksel, C. and Seiler, L. (2020). Quadratic Approximation of Cubic Curves. Proceedings of the ACM on Computer Graphics and Interactive Techniques, [online] 3(2),
    pp.1–17. doi:https://doi.org/10.1145/3406178.

    For the algorithm used to approximate cubic bezier curves as pairs of quadratic ones.

-   MDN Web Docs. (n.d.). SVG: Scalable Vector Graphics. [online] Available at: https://developer.mozilla.org/en-US/docs/Web/SVG.

    For providing a comprehensive overview of the SVG specifications, used in parsing `.svg` XML files.

-   Archive.org. (2025). Chapter 14 - OpenGL Programming Guide. [online] Available at:
    https://web.archive.org/web/20240118160026/http://www.glprogramming.com/red/chapter14.html#name13.

    For providing the algorithm to render arbitrary, concave polygons defined by `n` points in `O(n)` time. This algorithm has been extended in this library to support curved edges
    defined by quadratic beziers.

-   _The authors, contributors, and engineers who have written the following..._

    -   `bgfx`

        Is what we use for all 2D and 3D graphics.

    -   `glm`

        For all our numerical math, including 2D and 3D matrix transformations required in rendering.

    -   `libcxxext`

        Small C++ additions that are lightly sprinkled throughout our code, to make a faster, safer, better library.

    -   `moodycamel::ConcurrentQueue`

        A general-purpose, multi-producer, multi-consumer, thread-safe queue, used mainly to communicate sequences of events between threads.

    -   `pugixml`

        Loads `.svg` files for us, and other XML files if you'd like!

    -   `robin-hood-hashing`

        An (although no longer in development), very fast set of hashmaps, that we prefer using.

    -   `SDL3`

        For window management, input handling, and much more!

    -   `stb`

        We use `stb_truetype` for the loading of OpenType fonts, and `stb_image` to load `.png` files.

    -   `std_function`

        An alternate implementation of a general-purpose function wrapper which is measurably faster than the common standard implementations. We like that.
