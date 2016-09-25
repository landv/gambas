# gb.deg.unittest – A Gambas Unittest

A Gambas component for unittesting and test-driven programming. Forked and inspired from quite an old program: [COMUnit](http://comunit.sourceforge.net) and mainly JUnit. Currently beta state. With an unittest component one can develop software in a test-driven matter and is able to ensure that on refactoring the desired results of methods and classes stay the same.

Scroll down to understand how it works.

## Runner

Here you can see the Unittest tests itself. The testclasses (here called TestContainer) have to produce some failures and one error, to prove all is working fine.

![Unittest Runner](runner-screen.png)

## Tracer

The Trace tab gives a quick overview:

![Unittest Tracer](tracer-screen.png)


## How it works

It's a component. To make it work, you have to generate an installation package for your distribution with Gambas3 (it's proved to work for Version 3.8 upwards) and install it on your Linux system. After that you can use it in all your projects as a component.

The following example you also find in [this simple Gambas project](unittesthelloworld-0.0.1.tar.gz).

### Example TestContainer

Start by creating a class with a name like "_Test", for example "_TestHelloWorld". This class contains one or more public testmethod(s). It has to inherit from ATestContainer and it has to be exported to ensure that Unittest will recognize it. The trailing underscore ensures that the Gambas interpreter hides these classnames, even when they are exported. But you can take any other name for it. This class is the so called TestContainer:

----
    ' Gambas class file
    ''' TestContainer _TestHelloWorld
    
    Export
    Inherits ATestContainer
        
    Public Sub TestHelloWorld()
    
        Me.Result.AssertEqualsString("Hello World", Hello.World(), "Strings should be equal")
    
    End
----

### File Unittests
In your project's path a file called "Unittests" must exist. This file contains one line per TestContainer with the TestContainer's name:

    # File Unittests
    _TestHelloWorld

If it does not exist, the TestContainers are not found.

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
        
        Dim U as New Unittest
        
        U.Test
        
    
    End

----

If you did all this correctly and now hit < F5 >, Gambas will execute the startfunction in module Test, which works through the method(s) of our TestContainer and presents the test result in the console:

    ----------------------- Test Results ----------------------- 
     1 Tests done
    ------------------------------------------------------------ 
     No Errors
    
    
     No Failures
    ------------------------- Test End -------------------------
     Success!
    
----

Sooo ... and if you want to see the beautiful form, alter the startclass (erm ... startmodule)

----
    Public Sub Main()
    
      Dim U As New Unittest
      
      'U.Test is now replaced by
      U.ShowTestForm
    
    End
----

If you now hit < F5 > you'll see the testform where you can choose and run your tests. Afterwards have a look at the Trace tab:


![Unittest Tracer](trace-helloworld.png)

## Unittesting with gb.deg.unittest

Look around by positioning the cursor on "Unittest" and hit < F1 >. The relevant methods to test your code are in the class TestResult:

AddError AddFailure AddTrace Assert AssertEmpty AssertEqualsFloat AssertEqualsLong AssertEqualsObject AssertEqualsString AssertEqualsVariant AssertError AssertExists AssertFalse AssertNotEmpty AssertNotNull

Dig it out!

## Test fixture

Sometimes it is neccessary to create a "fixture", a special environment for a test or a couple of tests, and to destroy this environment after the test is done. For example a database connection should be established, some tables for testing should be created and this has to be reverted afterwards. This can be done with Setup... and Teardown... functions inside the TestContainer.

### Sub SetupEach() and Sub TeardownEach()

You can create methods with these names to create an environment for each testmethod before it is invoked and to destroy it afterwards. If you have five testmethods inside your TestContainer these functions will be invoked five times, SetupEach() before each testmethod, TeardownEach() after each testmethod. Got it?

### Sub SetupContainer() and Sub TeardownContainer()

You can create methods with these names to create an environment for all testmethods inside a TestContainer, in the beginning SetupContainer() is invoked and after all testmethods inside the testclass are done you can destroy the environment with TeardownContainer().


