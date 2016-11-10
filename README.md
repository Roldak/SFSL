SFSL
====
*(To be renamed Soon™)*

A statically typed and functionnal scripting language.

##Compiling and Running SFSL source code##

* Compiling: `sfslc source.sfsl` (only single source file for now)
* Running: no interpreter / VM on this repo. I have a basic interpreter running on the backend AST, but running is not the priority for me yet.

##Building the projects##

The *compiler library*, *user api*, *compiler executable*, *compiler tests*, *completer executable* and *sandard library* are all defined in the `CMakeLists.txt`. It shouldn't be too hard to build them since they only depend on the C++ standard library. (Need a compiler with C++11 support)

##Example##

Example using some features of the language, such as:
* Generics.
* Higher-order kinded types.
* Classes, inheritance, virtual methods.
* Type inference, type argument inference.

```
module test {
	using sfsl.lang
	
	/* Represents a generic mappable type.
	 * Parameter C has a higher-order kind and is used to abstract over the resuting collection type.
	 */
	abstract class Mappable[T, C: *->*] {
		abstract def map: [K](T->K)->C[K]
	}
	
	/* Basic list class.
	 * Only single inheritance is supported for now.
	 */
	abstract class List[T] : Mappable[T, List] {
		def prepend(x: T) => Node(x, this)
		
		abstract def foreach: (T->unit)->unit
		
		class Node(x: T, xs: List[T]) : List[T] {
			// redef is used to indicate an override def
			redef foreach(f: T->unit) => {
				f(x);
				xs.foreach(f);
			}
			redef map[K](f: T->K) => List[K].Node(f(x), xs.map(f))
		}
		
		class Leaf() : List[T] {
			redef foreach(unused_f: T->unit) => {}
			redef map[K](unused_f: T->K) => List[K].Leaf()
		}
	}
	
	/* Symbols that are marked `extern` are resolved at link time */
	extern def toString: int->string
	extern def println: string->unit
	extern def something: (int, int)->int
	
	/* The `entry` annotation is used to indicate that the following function is the entry point */
	@entry def main() => {
		x := List[int].Leaf().prepend(2).prepend(12).prepend(14);
		
		// Notice type argument inference (no need to write x.map[int])
		// as well as parameter type inference (no need to write (x: int) => ...)
		x.map(x => something(x, 3)).map(toString).foreach(println);
	}
}
```

This compiles to:

```
VISIBLE {
    global $ENTRY_POINT$ = new <anonymous function>$77
}

HIDDEN {
    class unit$0(0 fields) {
    }
    class bool$2(0 fields) {
    }
    class int$4(0 fields) {
    }
    class real$6(0 fields) {
    }
    class string$8(0 fields) {
    }
    class Box$10(1 fields) {
    }
    class Func0$13(0 fields) {
        meth $ABSTRACT_METHOD$
    }
    class Func1$16(0 fields) {
        meth $ABSTRACT_METHOD$
    }
    class Func2$19(0 fields) {
        meth $ABSTRACT_METHOD$
    }
    class Func3$22(0 fields) {
        meth $ABSTRACT_METHOD$
    }
    class Func4$25(0 fields) {
        meth $ABSTRACT_METHOD$
    }
    class Func5$28(0 fields) {
        meth $ABSTRACT_METHOD$
    }
    class Func6$31(0 fields) {
        meth $ABSTRACT_METHOD$
    }
    class Func7$34(0 fields) {
        meth $ABSTRACT_METHOD$
    }
    class Func8$37(0 fields) {
        meth $ABSTRACT_METHOD$
    }
    class Func9$40(0 fields) {
        meth $ABSTRACT_METHOD$
    }
    class Mappable$43(0 fields) {
        meth $ABSTRACT_METHOD$
    }
    meth new$51(3 vars) => {
        {
            ($0).$1 = ($1);
            ($0).$2 = ($2);
        };
        $0;
    }
    meth <anonymous function>$53(2 vars) => {
        ($1)[0](($0).$1);
        (($0).$2)[2]($1);
    }
    meth <anonymous function>$55(2 vars) => (((new Node$59)[5](($0).$0, $0))[3]($0))[4](($1)[0](($0).$1), (($0).$2)[0]($1))
    meth Node$init$57(3 vars) => {
        ($0).$3 = ($1);
        ($0).$4 = ($2);
        $0;
    }
    class Node$59(5 fields) : List$50 {
        meth <anonymous function>$55
        meth <anonymous function>$45
        meth <anonymous function>$53
        meth List$init$48
        meth new$51
        meth Node$init$57
    }
    meth new$61(1 vars) => {
        ();
        $0;
    }
    meth <anonymous function>$63(2 vars) => ()
    meth <anonymous function>$65(2 vars) => (((new Leaf$69)[5](($0).$0, $0))[3]($0))[4]()
    meth Leaf$init$67(3 vars) => {
        ($0).$1 = ($1);
        ($0).$2 = ($2);
        $0;
    }
    class Leaf$69(3 fields) : List$50 {
        meth <anonymous function>$65
        meth <anonymous function>$45
        meth <anonymous function>$63
        meth List$init$48
        meth new$61
        meth Leaf$init$67
    }
    meth <anonymous function>$45(2 vars) => (((new Node$59)[5](($0).$0, $0))[3]($0))[4]($1, $0)
    meth List$init$48(2 vars) => {
        ($0).$0 = ($1);
        $0;
    }
    class List$50(1 fields) : Mappable$43 {
        meth $ABSTRACT_METHOD$
        meth <anonymous function>$45
        meth $ABSTRACT_METHOD$
        meth List$init$48
    }
    meth ()$72(2 vars) => (test.something:(sfsl.lang.int, sfsl.lang.int)->sfsl.lang.int)[0]($1, 3)
    class <anonymous function>$74(0 fields) : Func1$16 {
        meth ()$72
    }
    meth ()$75(2 vars) => {
        $1 = (((((((new Leaf$69)[5](($0).$0, $0))[3]($0))[4]())[1](2))[1](12))[1](14));
        ((($1)[0](new <anonymous function>$74))[0](test.toString:(sfsl.lang.int)->sfsl.lang.string))[2](test.println:(sfsl.lang.string)->sfsl.lang.unit);
    }
    class <anonymous function>$77(0 fields) : Func0$13 {
        meth ()$75
    }
}
```

It takes `0.1` seconds on my machine (*debug build*). 
Compilation steps:
* `NameAnalysisPhase`: Generates scopes, allocate symbols and assigns them to the symbolics.
* `KindCheckingPhase`: Assigns kinds to every kinded nodes, and reports any kind check errors.
* `TypeCheckingPhase`: Assigns types to every typed nodes, and reports any type check errors.
* `UsageAnalysisPhase`: Checks whether variables are well declared, initialized before being used, etc.
* `PreTransformPhase`: Assigns useful informations to node and performs last minute operations.
* `AST2BASTPhase`: Transforms the frontent AST into the backend AST. (*which can be seen above*)

This is another type of AST that can be either interpreted directly, or compiled down to some homemade bytecode. (WIP)

###Tutorial###

Now let's see how to integrate *SFSL* to your C++ project:
The first step is of course to include `sfsl.h` (you can copy the `include` directory of the SFSL project to your project or simply add this directory as an include directory in your project's build configuration).

You will then be able to instantiate an `sfsl::Compiler` object:
```
Compiler cmp(CompilerConfig().with<opt::Reporter>(StandartReporter::CerrReporter));
```
Several options can be given to `CompilerConfig` to: print memory usage, print compilation time, etc.

We can now use our `Compiler` object to parse the source code that you want:
```
ProgramBuilder builder = cmp.parse(sourceName, sourceContent);
```
The `ProgramBuilder` object allows your to navigate through modules of your source, define extern functions, define classes, etc. Let's try to add a function `f` of type `int->int` in module `example`:
```
builder.openModule("example").externDef("f", cmp.parseType("int->int"));
```
Function `f` is then available at path `example.f` in the source code! This means that in `sourceContent`, we could have somewhere `println(example.f(2));` which will compile and typecheck correctly. An error would of course have been reported if we had not define `example.f` neither directly in the source code nor from the C++ `ProgramBuilder`.

We now need to define a compilation pipeline, through which the program will flow:
```
Pipeline ppl = Pipeline::createDefault();
```
You can either start with a default pipeline (which does the typical *Name analysis*, *Type checking*, etc., or an empty one if you want to build it from scratch. At this point, you can insert your own phases into the pipeline. Check out `samples/API/SamplePhases.cpp` for examples of custom phases.
Now we must create a *collector* object which will collect the output of the compilation. There will soon be the *VMCollector* which will basically create the default SFSL virtual machine from the output bytecode. For now, let's use an `EmptyCollector`, which does not collect anything.
```
EmptyCollector emc;
```
We are now ready to compile!
```
cmp.compile(builder, emc, ppl);
```
Well, we will have nothing to retrieve from our collector, but it's coming soon! :)
The interface will probably look like:
```
vm.link(example_f_symbol, [](ExecCtx& ctx) { return ctx.pushInt(2 * ctx.popInt(0)); });
...
vm.call(program_main_symbol, ...);
```
Where `example_f_symbol` and `program_main_symbol` are strings that fully qualify a *def* symbol. Their format is:  `"path_to_def_symbol:type_of_def_symbol"`. For example, `example_f_symbol` is `"example.f:(sfsl.lang.int)->sfsl.lang.int"`. Of course you won't have to write them yourselves, they will be provided to you someway when you call `externDef` & co.
