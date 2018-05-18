# gb.unittest – A Gambas Unittest

A Gambas component for unittesting and test-driven programming. Forked and inspired from quite an old program: [COMUnit](http://comunit.sourceforge.net) and mainly JUnit. Currently alpha state. With an unittest component one can develop software in a test-driven matter and is able to ensure that on refactoring the desired results of methods and classes stay the same.

## How it works

It's a component. To make it work, you have to generate an installation package for your distribution with Gambas3 (it's proved to work for Version 3.8 upwards) and install it on your Linux system. After that you can use it in all your projects as a component.

The following example you also find in [this simple Gambas project](unittesthelloworld-0.0.1.tar.gz).

### Example TestContainer

Start by creating a class with a name like "_Test", for example "_TestHelloWorld". This class contains one or more public testmethod(s). It has to inherit from UnitTest and it has to be exported to ensure that Unittest will recognize it. The trailing underscore ensures that the Gambas interpreter hides these classnames, even when they are exported. But you can take any other name for it. This class is the so called TestContainer:

----
    ' Gambas class file
    ''' TestContainer _TestHelloWorld
    
    Export
    Inherits UnitTest
        
    Public Sub TestHelloWorld()
    
        Me.Result.AssertEqualsString("Hello World", Hello.World(), "Strings should be equal")
    
    End
----

### Module(Function) to test:

To make it work, we need a funktion to test. So we create a function "World" in a module "Hello" in our project:

----

    ' Gambas module file
    
    ''' Module is named "Hello"
    
    Public Function World() As String
    
      Dim w As String
    
      w = "Hello World"
      Return w
    
    End
    
----

### Invite Unittest

The simple way to execute the Unittest is to create another module, name it "Test" or something more interesting and make it a Gambas Startclass:

----

    'Module Test
    'Starts the Unittest
    
    Public Sub Main()
        
        Unittest.Run()
    
    End

----

If you did all this correctly and now hit &lt;F5&gt;, Gambas will execute the startfunction in module Test, which works through the method(s) of our TestContainer and presents the test result in the console:

    ----------------------- Test Results ----------------------- 
     1 Tests done
    ------------------------------------------------------------ 
     No Errors
    
    
     No Failures
    ------------------------- Test End -------------------------
     Success!
    
----

## Unittesting with gb.unittest

Look around by positioning the cursor on "Unittest" and hit &lt;F1&gt;. The relevant methods to test your code are in the class TestResult:

AddError AddFailure AddTrace Assert AssertEmpty AssertEqualsFloat AssertEqualsLong AssertEqualsObject AssertEqualsString AssertEqualsVariant AssertError AssertExists AssertFalse AssertNotEmpty AssertNotNull

Dig it out!

## Test fixture

Sometimes it is neccessary to create a "fixture", a special environment for a test or a couple of tests, and to destroy this environment after the test is done. For example a database connection should be established, some tables for testing should be created and this has to be reverted afterwards. This can be done with Setup... and Teardown... functions inside the TestContainer.

### Sub SetupEach() and Sub TeardownEach()

You can create methods with these names to create an environment for each testmethod before it is invoked and to destroy it afterwards. If you have five testmethods inside your TestContainer these functions will be invoked five times, SetupEach() before each testmethod, TeardownEach() after each testmethod. Got it?

### Sub SetupContainer() and Sub TeardownContainer()

You can create methods with these names to create an environment for all testmethods inside a TestContainer, in the beginning SetupContainer() is invoked and after all testmethods inside the testclass are done you can destroy the environment with TeardownContainer().


