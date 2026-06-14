this is not a showcase, its not supposed to be impressive. but ifkyk
# ~~sol32 Programming Toolchain~~
Processor design is hard, both in RTL and specifications. Decisions made in the design phase are permanent and effect the further development. Even with almost a half of a century of processor design history, there are some tradeoffs that make sense in one context and not in others. It is an extremely complex topic that requires experience, which I am slowly acquiring.

Like other architectures sol32 has its upsides and downsides, but in the end I figured that the downs overpowered the ups and abandoned the design. Alongside went the programming language Apogee, as I wanted to target sol32 as the one and only architecture it supported; using LLVM, transpiling or emitting x86/aarch assembly is trivial and components required for that are ready.

This repo contains the toolchain for sol32: an assembler, a preprocessor (macros/constants), and a recursive descent parser for a compiler frontend. It has no outside dependencies beyond Python's built-in re module and the C standard library.

## Why?
The main goal was to get a deeper, hands-on understanding of compilers and processor architecture.
Trying to do both at once was a dumb idea, and I crashed and burned but I still learned a ton, including:
- **parsing** and not just recursive descent. I have **wasted** a good chunk of my summer reading _The Dragon Book_ (and some others) rambling about academic concepts and providing insufficient context to make the topic seem more complex than it actually is. After learning that many successful compilers use hand written parsers I threw the PDFs into the trash and just wrote the thing. 
- **processor architectures**. I studied many existing architectures including x86, ARMv8 and RISC-V but also some of the embedded, dead and experimental ([ZipCPU](https://github.com/ZipCPU), [ForwardCom](https://forwardcom.info/)) ones to see what works and does not. This also made me pretty familiar with assembly programming.

And most importantly, for my GitHub profile to show off, although I am skeptical about the effectiveness.
