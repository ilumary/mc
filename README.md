# mc

mc is my take on the always popular minecraft clones and a follow-up to my last attempt at Vulkan. 

## Features (as of now)

- Fully functional vulkan render backend
- First-Person Camera with typical wasd controls 
- 3d chunk system
- Terrain generation with perlin noise

Planned: 

- Refactor and clean up code! 
- Fix Validation Error
- Figure out a way to reduce memory usage in high render distance scenarios
- Add placing/destruction of blocks
- add dynamic loading/unloading to chunk system
- Implement some kind of better lightning system
- Add water and corresponding physics 

## Tech

mc uses a number of third party libraries/headers:

- [vulkan] - Low level graphics API, in my case with molten-vk because Apple had to invent it's own graphics API
- [vk-bootstrap] - Helper library for setting up initial boilerplate vulkan code
- [VulkanMemoryAllocator] - Helper library for managing memory allocation in vulkan
- [glfw] - Platform independent window library
- [fmt] - Formatting library used for logging
- [glm] - Used for everything that includes some kind of math aka matrices and vectors
- [stb] - Used for texture images

## Installation

As of now mc is not very easy to get running, but I intend on changing that later down the road. For now either you use a Mac with homebrew or you will have to change a lot of the include paths in the cmake file. Only vk-bootstrap, stb and VulkanMemoryAllocator will work out of the box, because they were added with git submodules.

That aside, with the right include parameters, the code base itself should run on the major platforms without issues though this is in no way tested so don't expect too much. 

## Development

mc is as of now in active Development. 

I intend on steadily building it out, but I am unsure how far I will take this as it has more of a learning purpose to me than a professional one.

## License

You are free to do with my code whatever you want and if you have some suggestions or issues, let me know.

[vulkan]:<https://www.vulkan.org>
[vk-bootstrap]:<https://github.com/charles-lunarg/vk-bootstrap>
[VulkanMemoryAllocator]:<https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator>
[glfw]:<https://github.com/glfw/glfw>
[fmt]:<https://github.com/fmtlib/fmt>
[glm]:<https://github.com/g-truc/glm>
[stb]:<https://github.com/nothings/stb>
