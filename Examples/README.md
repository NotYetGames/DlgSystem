# Execute Blueprint and C++ functions through your dialogues!

It is now possible to execute Blueprint or C++ functions using your dialogues.
You can use functions for your `Enter Events`, `Enter Conditions` and your `Text Arguments`. 
You can even pass along variables onto your functions. This allows you to reuse the same function multiple times!

* [Setup](#setting-up-your-actors-and-dialogue-files)
* [Enter Events](#enter-events)
* [Enter Conditions](#enter-conditions)
* [Text Arguments](#text-arguments)
* [Supported input parameters](#supported-input-parameters)

## Setting up your actors and dialogue files <a name="setting-up-your-actors-and-dialogue-files"></a>

1. Make sure your Blueprint inherits from `DlgDialogueParticipant`

![setting up your Blueprint](Examples/Images/SettingUpActor.gif)

2. Make sure you assign a class in your dialogue after setting up your participant.  
  2.1 After writing your `Participant Name` you need to click on empty space in the dialogue editor to open the `Dialogue Data` window.

![setting up your dialogue](Examples/Images/SettingUpDialogue.gif)


3. After setting up your Blueprint and your dialogue, you have to setup the participant name in your Blueprint. This has to be the same name you used in your dialogue.

![setting up your participant name](Examples/Images/SettingUpParticipantName.gif)

## Enter Events <a name="enter-events"></a>

1. Setting up your Blueprint Enter Event functions _without_ input parameters is as simple as creating a new function _without_ input and output parameters.  
  1.1 Your function cannot have output parameters to be registered as an Enter Event.
  
![Setting up your Blueprint Enter Event](Examples/Images/EnterEvents/EnterEventBP.gif)

2. To run your Enter Event, you have to select the participant you want to run your function on (in our case `MyActor`). Then select the `Event Type` `Call function`. After that select the function name you wish to execute.

![Setting up your dialogue Enter Event](Examples/Images/EnterEvents/EnterEventDlg.gif)

3. In this example I open our newly created dialogue on the `BeginPlay`. It then prints `Hello` into the console. 

![Example usage of Enter Events](Examples/Images/EnterEvents/EnterEventExample.gif)

4. Setting up your Blueprint Enter Event functions _with_ input parameters is as simple as creating a new function _with_ input, but without output parameters.

![Setting up your Blueprint variable Enter Event](Examples/Images/EnterEvents/EnterEventVariablesBP.gif)

5. Using your Blueprint functions _with_ input parameters in your dialogue is as simple as creating a new Enter Event and selecting the `Event Type` `Call function with variables`. After that select the function name you wish to execute.

![Setting up your dialogue Enter Event with input variables](Examples/Images/EnterEvents/EnterEventVariablesDlg.gif)

6. In this example I open our newly created dialogue on the `BeginPlay`. It then prints `Goodbye` into the console. As that is the value we specified in the `Variables` field of your dialogue.

![Example usage of Enter Events with variables](Examples/Images/EnterEvents/EnterEventVariablesExample.gif)

## Enter Conditions <a name="enter-conditions"></a>

1. Setting up your Blueprint Enter Condition functions _without_ input parameters is as simple as creating a new function _without_ input, but *with* a `Boolean` output parameter.  
  1.1 Your function *must* have at least 1 _Boolean_ output parameter. Only the first parameter is used, and the rest will be ignored.
  
![Setting up your Blueprint Enter Condition](Examples/Images/EnterConditions/EnterConditionBP.gif)

2. To run your Enter Conditions, you have to select the participant you want to run your function on (in our case `MyActor`). Then select the `Condition Type` `Check Class Method`. After that select the function name you wish to execute.

![Setting up your dialogue Enter Condition](Examples/Images/EnterConditions/EnterConditionDlg.gif)

3. In this example I open our newly created dialogue on the `BeginPlay`. It then doesn't print anything in the console, as our function is returning `false`, but our dialogue expects `true`.

![Example usage of Enter Events](Examples/Images/EnterConditions/EnterConditionExample.gif)

3.1 In the next example I've changed it so our dialogue expects `false` as a value instead, and our dialogue now prints to the console again.

![Example usage of Enter Events](Examples/Images/EnterConditions/EnterConditionExample2.gif)

4. Setting up your Blueprint Enter Condition functions _with_ input parameters is as simple as creating a new function _with_ input and with a `Boolean` output parameter.

![Setting up your Blueprint variable enter event](Examples/Images/EnterConditions/EnterConditionVariablesBP.gif)

5. Using your Blueprint Enter Conditions _with_ input parameters in your dialogue is as simple as creating a new Enter Conditions and selecting the `Condition Type` `Check Class Method and Pass Variables`. After that select the function name you wish to execute.

![Setting up your dialogue Enter Condition with input variables](Examples/Images/EnterConditions/EnterConditionVariablesDlg.gif)

6. In this example I open our newly created dialogue on the `BeginPlay`. It then failes to open the dialogue because our Enter Condition has failed.

![Example usage of Enter Conditions with variables](Examples/Images/EnterConditions/EnterConditionVariablesExample.gif)

6.1 In the next example I've changed the values in the `Variables` field to be of the same value. This causes our Enter Condition to successfully run.

![Example usage of Enter Conditions with variables](Examples/Images/EnterConditions/EnterConditionVariablesExample2.gif)

## Text Arguments <a name="text-arguments"></a>

1. Setting up your Blueprint Text Arguments functions _without_ input parameters is as simple as creating a new function _without_ input, but *with* a `Text` output parameter
  1.1 Your function *must* have at least 1 _Text_ output parameter. Only the first parameter is used, and the rest will be ignored.
  
![Setting up your Blueprint Text Argument](Examples/Images/TextArguments/TextArgumentBP.gif)

2. To use your Text Argument, you have to first create a Text Argument in your Text field. Then select the participant you want to run your function on (in our case `MyActor`). And finally select the `Type` `Class Method Return Parameter`. After that select the function name you wish to execute.

![Setting up your dialogue Text Argument](Examples/Images/TextArguments/TextArgumentDlg.gif)

3. In this example I open our newly created dialogue on the `BeginPlay`. It then prints `Hello` + `my name` into the dialogue widget. (The dialogue widget has to be created by you).

![Example usage of Text Arguments](Examples/Images/TextArguments/TextArgumentExample.gif)

4. Setting up your Blueprint Text Arguments functions _with_ input parameters is as simple as creating a new function _with_ input and with a `Text` output parameter.

![Setting up your Blueprint variable Text Argument](Examples/Images/TextArguments/TextArgumentVariablesBP.gif)

5. Using your Blueprint Text Argument functions _with_ input parameters in your dialogue is as simple as creating a new Text Argument in your text field and selecting the `Type` `Class Method Return Parameter With Variables`. After that select the function name you wish to execute.

![Setting up your dialogue Text Argument with input variables](Examples/Images/TextArguments/TextArgumentVariablesDlg.gif)

6. In this example I open our newly created dialogue on the `BeginPlay`. It then prints `Is 5 higher then 10?` + `false` into the dialogue widget. It prints false, because that is the output our function returns.

![Example usage of Text Arguments with input variables](Examples/Images/TextArguments/TextArgumentVariablesExample.gif)

## Supported Input Parameters <a name="supported-input-parameters"></a>

Pretty much every value is supported, as long as you know how to type your value as a string. 
For example, converting a `Boolean` into a string would require you to type `true` or `false`.
`Integers` and `Floats` are supported by default.

`Enums` would require you to make use of the display name to convert it from a string back into an enum.

Basically, as long as you can convert your value from a string into the desired result, you can make use of it!





