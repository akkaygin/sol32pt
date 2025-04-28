# ~~sol32 Programming Toolchain~~
Processor design is hard, both in RTL and specifications. Decisions made in the design phase are permanent and effect the further development. Even with almost a half of a century of processor design history, there are some tradeoffs that make sense in one context and not in others. It is an extremely complex topic that requires experience, which I am slowly acquiring.

Like other architectures sol32 has its upsides and downsides, but in the end I figured that the downs overpowered the ups and abandoned the design. Alongside went the programming language Apogee, as I wanted to target sol32 as the one and only architecture it supported; using LLVM, transpiling or emitting x86/aarch assembly is trivial and components required for that are ready.

This repo contains the toolchain for sol32: an assembler, a preprocessor (macros/constants), and a recursive descent parser for a compiler frontend. It has no outside dependencies beyond Python's built-in re module and the C standard library.

My processor design adventure isn't over, though. I'm working on a new design that's leaner and better defined. It's private for now but will go public once I polish some rough edges. Stay tuned.

## Why?
The main goal was to get a deeper, hands-on understanding of compilers and processor architecture.
Trying to do both at once was a dumb idea, and I crashed and burned but I still learned a ton, including:
- **parsing** and not just recursive descent. I have **wasted** a good chunk of my summer reading _The Dragon Book_ (and some others) rambling about academic concepts and providing insufficient context to make the topic seem more complex than it actually is. After learning that many successful compilers use hand written parsers I threw the PDFs into the trash and just wrote the thing. 
- **processor architectures**. I studied many existing architectures including x86, ARMv8 and RISC-V but also some of the embedded, dead and experimental ([ZipCPU](https://github.com/ZipCPU), [ForwardCom](https://forwardcom.info/)) ones to see what works and does not. This also made me pretty familiar with assembly programming.

And most importantly, for my GitHub profile to show off, although I am skeptical about the effectiveness.

## Components
* **The Assembler:** The core of the project. It can handle variables, labels and of course, instructions to produce valid machine code for _some_ sol32 variant, which is the reason why it failed.
* **Tokenizer:** Not a lot to say, this module breaks down the input assembly code into a stream of tokens, forming the foundation for syntactic analysis.
* **Recursive Descent Parser:** The parser consumes the token stream and builds an Abstract Syntax Tree (AST). The AST represents the hierarchical structure of the source code program, making it easier for subsequent processing like code generation, but I do not have a backend because I do not have a language specification.
* **Macro and Constant Preprocessor:** This Python script uses regular expressions to handle simple macro expansions and constant substitutions before the assembly code is fed into the assembler. Regex is the main bottleneck here as I cannot manage state, I am integrating this feature into the new compiler, like many modern programming languages do.

## ~~Building and Running~~
Don't.
You can build this using the makefile, but there is no reason to. Just look at the source code or something.
