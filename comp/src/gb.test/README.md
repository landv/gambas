# gb.test – A Gambas Unittest

A Gambas component for unittesting and test-driven programming. Inspired from quite an old program: [COMUnit](http://comunit.sourceforge.net) and other test frameworks. Currently alpha state. With an unittest component you can develop software in a test-driven matter (write test first, program functionality afterwards) and you are able to ensure that on refactoring the desired results of methods and classes stay the same.

Tests are output as [Tap](https://testanything.org/ testanything.org) so that they can be displayed, analyzed or viewed with any [Tap consumer](https://testanything.org/consumers.html).

## How it works

It's a component. To make it work, you have to generate an installation package (innside the project type Ctrl-Alt-I) for your distribution with Gambas3 (it's proved to work for Version 3.11 upwards) and install it on your Linux system. After that you can use it in all your projects as a component.

There is an example in [this simple Gambas project](unittesthelloworld-0.0.1.tar.gz).

### Example TestContainer

Start by creating a class with a name like "Test", for example "TestHelloWorld". This class contains one or more public testmethod(s). It has to inherit from UnitTest and as long as Gambas itself does not have a proper functionality to recognize Test classes, it has to be exported to ensure that the test classes can be executed (maybe this will be changed in future). The trailing underscore ensures that the Gambas IDE hides these classnames, even when they are exported. But you can take any other name for it. This class is the so called TestContainer:

----
    ' Gambas class file
    ''' TestContainer TestHelloWorld

    Export
    Inherits UnitTest

    Public Sub TestHelloWorld()

        Assert.EqualsString("Hello World", Hello.World(), "Strings should be equal")

    End
----

### Module(Function) to test:

To make it work, we need a funktion that will be tested. So we create a function "World" in a module "Hello" in our project:

----

    ' Gambas module file

    ''' Module is named "Hello"

    Public Function World() As String

      Return "Hello World"

    End

----

###  Bringing Unittest into play

A simple way to execute the Unittest is to create another module, name it "Test" or something more interesting and make it a Gambas Startclass:

----

    'Module Test

    'Is a startclass
    'Starts the Unittest, when F5 was hit

    Public Sub Main()

        Unittest.Main()

    End

----

If you did all this correctly and now hit &lt;F5&gt;, Gambas will execute the startfunction in module Test, which works through the method(s) of our TestContainer and presents the test result in the console:

    ok 1 - Test Hello.FortyTwo
    ok 2 - Test equal strings just for fun
    ok 3 - HW strings should be equal
    not ok 4 - Test Hello.Right will go wrong
    # In TestHello:TestRight
    
    # Failed tests: 4
    # 1..4
    # ------- No success!  -------

In this example you see, that a failure occurs. I you want to debug the code you can set a breakpoint inside TestHello.TestRight, hit &lt;F5&gt; again and start debugging.

If you have a lot of tests, and want to let run just one, you can do that like so:

----

    'Module Test

    'Is a startclass
    'Starts the Unittest, when F5 was hit

    Public Sub Main()

        Unittest.Main("TestHello", "TestRight")

    End

----

## Test your project on the console

If you made an executable of your project, you can even test it on the console. The command **gbr3 -s "UnitTest" unittesthelloworld.gambas** executes the unittests and prints the result to standard output:

    ok 1 - Test Hello.FortyTwo
    ok 2 - Test equal strings just for fun
    ok 3 - HW strings should be equal
    not ok 4 - Test Hello.Right will go wrong
    # In _TestHello:TestRight
    
    # Failed tests: 4
    # 1..4
    # ------- No success!  -------


## Test fixture

Sometimes it is neccessary to create a "fixture", a special environment for a test or a couple of tests, and to destroy this environment after the test is done. For example a database connection should be established, some tables for testing should be created and this has to be reverted afterwards. This can be done with Setup... and Teardown... functions inside the TestContainer.

### Sub SetupEach() and Sub TeardownEach()

You can create methods with these names to create an environment for each testmethod before it is invoked and to destroy it afterwards. If you have five testmethods inside your TestContainer these functions will be invoked five times, SetupEach() before each testmethod, TeardownEach() after each testmethod. Got it?

### Sub SetupContainer() and Sub TeardownContainer()

You can create methods with these names to create an environment for all testmethods inside a TestContainer, in the beginning SetupContainer() is invoked and after all testmethods inside the testclass are done you can destroy the environment with TeardownContainer().