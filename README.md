SFSL
====
*(Temporary name to compensate for the lack of inspiration)*

A statically typed and functionnal scripting language.

##Example##

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

##Compiling and Running SFSL source code##

* Compiling: `sfslc -s source.sfsl` (only single source file for now)
* Running: no interpreter / VM on this repo. I have a basic interpreter running on the backend AST, but running is not the priority for me yet.

##Building the projects##

The compiler *library*, *user api*, *compiler executable*, *compiler tests*, *completer executable* and *sandard library* are all defined in the `CMakeLists.txt`. It shouldn't be too hard to build them since they only depend on the C++ standard library. (Need a compiler with C++11 support)
