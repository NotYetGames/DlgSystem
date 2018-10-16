// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once
#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Layout/Margin.h"

#include "DlgIDialogueEditorModule.h"

#include "DlgSystemSettings.generated.h"

/**
 *  Defines the format of the Dialogue text
 */
UENUM()
enum class EDlgDialogueTextFormat : uint8
{
	/** No Text Format used. Only the uasset */
	DlgDialogueNoTextFormat			UMETA(DisplayName = "No Text Format"),

	/** The own Dialogue Text format */
	DlgDialogueTextFormatDialogue	UMETA(DisplayName = "DlgText"),

	/** The JSON format. */
	DlgDialogueTextFormatJson		UMETA(DisplayName = "JSON"),

	/** Hidden, represents the number of text formats */
	DlgDialogueTextFormat_Num 		UMETA(Hidden),
};

/**
 *  Defines the displayed properties of the voice fields.
 */
UENUM()
enum class EDlgVoiceDisplayedFields : uint8
{
	/** No Voice fields are displayed. */
	DlgVoiceDisplayedNoVoice					UMETA(DisplayName = "No Voice"),

	/** Only display the SoundWave voice fields. */
	DlgVoiceDisplayedSoundWave					UMETA(DisplayName = "Sound Wave"),

	/** Only display the DialogueWave voice fields. */
	DlgVoiceDisplayedDialogueWave				UMETA(DisplayName = "Dialogue Wave"),

	/** Display both SoundWave and DialogueWave fields. */
	DlgVoiceDisplayedSoundWaveAndDialogueWave	UMETA(DisplayName = "Sound Wave & Dialogue Wave")
};

/**
 *  Defines the visibility of the SpeakerState values
 */
UENUM()
enum class EDlgSpeakerStateVisibility : uint8
{
	/** No visibility fields are displayed. */
	DlgHideAll					UMETA(DisplayName = "Hide All"),

	/** Only display the SoundWave voice fields. */
	DlgShowOnEdge				UMETA(DisplayName = "Show On Edge"),

	/** Only display the DialogueWave voice fields. */
	DlgShowOnNode				UMETA(DisplayName = "Show On Node"),

	/** Display both SoundWave and DialogueWave fields. */
	DlgShowOnNodeAndEdge		UMETA(DisplayName = "Show On Both")
};

/**
 * Defines what key combination to press to add a new line to an FText
 */
UENUM()
enum class EDlgTextInputKeyForNewLine : uint8
{
	/** Press 'Enter' to add a new line. */
	DlgTextInputKeyForNewLineEnter					UMETA(DisplayName = "Enter"),

	/** Presst 'Shift + Enter' to add a new line. (like in blueprints) */
	DlgTextInputKeyForNewLineShiftPlusEnter			UMETA(DisplayName = "Shift + Enter"),
};

// Config = DlgSystemPlugin, DefaultConfig
// UDeveloperSettings classes are autodiscovered https://wiki.unrealengine.com/CustomSettings
UCLASS(Config = EditorPerProjectUserSettings, DefaultConfig, meta = (DisplayName = "Dialogue Editor Settings"))
class DLGSYSTEM_API UDlgSystemSettings : public UDeveloperSettings
{
	GENERATED_BODY()

	typedef UDlgSystemSettings Self;

public:
	UDlgSystemSettings();

	// UDeveloperSettings interface
	/** Gets the settings container name for the settings, either Project or Editor */
	FName GetContainerName() const override { return "Project"; }
	/** Gets the category for the settings, some high level grouping like, Editor, Engine, Game...etc. */
	FName GetCategoryName() const override { return "Editor"; };
	/** The unique name for your section of settings, uses the class's FName. */
	FName GetSectionName() const override { return Super::GetSectionName(); };

#if WITH_EDITOR
	/** Gets the section text, uses the classes DisplayName by default. */
	FText GetSectionText() const override;
	/** Gets the description for the section, uses the classes ToolTip by default. */
	FText GetSectionDescription() const override;

	/** Whether or not this class supports auto registration or if the settings have a custom setup */
	bool SupportsAutoRegistration() const override { return true; }

	// UObject interface
	bool CanEditChange(const UProperty* InProperty) const override;
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

	// Own functions
#define CREATE_SETTER(_NameMethod, _VariableType, _VariableName)  \
	void _NameMethod(_VariableType InVariableValue)               \
	{                                                             \
		if (_VariableName != InVariableValue)                     \
		{                                                         \
			_VariableName = InVariableValue;                      \
			SaveSettings();                                       \
		}                                                         \
	}

	CREATE_SETTER(SetShowPrimarySecondaryEdges, bool, bShowPrimarySecondaryEdges)
	CREATE_SETTER(SetDrawPrimaryEdges, bool, bDrawPrimaryEdges)
	CREATE_SETTER(SetDrawSecondaryEdges, bool, bDrawSecondaryEdges)
	CREATE_SETTER(SetHideEmptyDialogueBrowserCategories, bool, bHideEmptyDialogueBrowserCategories)

#undef CREATE_SETTER

	/** Saves the settings to the config file depending on the settings of this class. */
	void SaveSettings()
	{
		const UClass* ThisClass = GetClass();
		if (ThisClass->HasAnyClassFlags(CLASS_DefaultConfig))
		{
			UpdateDefaultConfigFile();
		}
		else if (ThisClass->HasAnyClassFlags(CLASS_GlobalUserConfig))
		{
			UpdateGlobalUserConfigFile();
		}
		else
		{
			SaveConfig();
		}
	}

public:
	// Some constants used. TODO make these configurable
	static const FText EdgeTextFinish;
	static const FText EdgeTextNext;

public:
	/** If enabled this clears the dialogue history automatically on Editor Start PIE and On Load New Map */
	UPROPERTY(Category = "Runtime", Config, EditAnywhere)
	bool bClearDialogueHistoryAutomatically = true;

	/** The dialogue text format used for saving and reloading from text files. */
	UPROPERTY(Category = "Dialogue", Config, EditAnywhere, DisplayName = "Text Format")
	EDlgDialogueTextFormat DialogueTextFormat = EDlgDialogueTextFormat::DlgDialogueTextFormatDialogue;

	/** What Voice fields to show in the Dialogue Editor, if any. */
	UPROPERTY(Category = "Dialogue", Config, EditAnywhere, DisplayName = "Displayed Voice Fields")
	EDlgVoiceDisplayedFields DialogueDisplayedVoiceFields = EDlgVoiceDisplayedFields::DlgVoiceDisplayedSoundWave;

	/** Where to display the SpeakerState FName property */
	UPROPERTY(Category = "Dialogue", Config, EditAnywhere, DisplayName = "SpeakerState Visibility")
	EDlgSpeakerStateVisibility DialogueSpeakerStateVisibility = EDlgSpeakerStateVisibility::DlgHideAll;

	/** What key combination to press to add a new line for FText fields in the Dialogue Editor. */
	UPROPERTY(Category = "Dialogue", Config, EditAnywhere, DisplayName = "Text Input Key for NewLine")
	EDlgTextInputKeyForNewLine DialogueTextInputKeyForNewLine = EDlgTextInputKeyForNewLine::DlgTextInputKeyForNewLineEnter;

	/** Any properties that belong to these classes wont't be shown in the suggestion list when you use the reflection system (class variables). */
	UPROPERTY(Category = "Dialogue", Config, EditAnywhere)
	TArray<UClass*> BlacklistedReflectionClasses;

	/** Whether the description text wraps onto a new line when it's length exceeds this width; if this value is zero or negative, no wrapping occurs. */
	UPROPERTY(Category = "Graph Node", Config, EditAnywhere)
	float DescriptionWrapTextAt = 256.f;

	/** The amount of blank space left around the edges of the description text area. */
	UPROPERTY(Category = "Graph Node", Config, EditAnywhere)
	FMargin DescriptionTextMargin = FMargin(5.f);

	/** The horizontal alignment of the graph node title and icon */
	UPROPERTY(Category = "Graph Node", Config, EditAnywhere)
	TEnumAsByte<EHorizontalAlignment> TitleHorizontalAlignment = HAlign_Fill;

	/** To show or not the condition icon in the case the node has any enter conditions defined on it. */
	UPROPERTY(Category = "Graph Node", Config, EditAnywhere)
	bool bShowHasEnterConditionsIcon = true;

	/** To show or not the event icon in the case the node has any enter events defined on it. */
	UPROPERTY(Category = "Graph Node", Config, EditAnywhere)
	bool bShowHasEnterEventsIcon = true;

	/** To show or not the voice icon in the case the node has any voice members set */
	UPROPERTY(Category = "Graph Node", Config, EditAnywhere)
	bool bShowHasVoiceIcon = true;

	// Colors based on https://material.io/guidelines/style/color.html#color-color-palette

	/** The background color of the normal speech node. */
	UPROPERTY(Category = "Graph Node", Config, EditAnywhere)
	FLinearColor SpeechNodeColor = FLinearColor{0.050980f, 0.278431f, 0.631373f, 1.f}; // blueish

	/** The background color of the root node. */
	UPROPERTY(Category = "Graph Node", Config, EditAnywhere)
	FLinearColor RootNodeColor = FLinearColor{0.105882f, 0.368627f, 0.125490f, 1.f}; // greenish

	/** The background color of the end node. */
	UPROPERTY(Category = "Graph Node", Config, EditAnywhere)
	FLinearColor EndNodeColor = FLinearColor{0.835294f, 0.f, 0.f, 1.f}; // redish

	/** The background color of the virtual parent node. */
	UPROPERTY(Category = "Graph Node", Config, EditAnywhere)
	FLinearColor VirtualParentNodeColor = FLinearColor{0.129412f, 0.129412f, 0.129412f, 1.0f}; // dark gray

	/** The background color of the selector first node. */
	UPROPERTY(Category = "Graph Node", Config, EditAnywhere)
	FLinearColor SelectorFirstNodeColor = FLinearColor{0.f, 0.721569f, 0.831373f, 1.f};  // cyan

	/** The background color of the selector random node. */
	UPROPERTY(Category = "Graph Node", Config, EditAnywhere)
	FLinearColor SelectorRandomNodeColor = FLinearColor{1.f, 0.839216f, 0.f, 1.f}; // yellow

	/** The background color of the selector random node. */
	UPROPERTY(Category = "Graph Node", Config, EditAnywhere)
	FLinearColor SpeechSequenceNodeColor = FLinearColor{0.050980f, 0.278431f, 0.631373f, 1.f}; // blueish

	/** The background color of the node borders. */
	UPROPERTY(Category = "Graph Node", Config, EditAnywhere)
	FLinearColor BorderBackgroundColor = FLinearColor::Black;

	/** The background color of the node borders when hovered over */
	UPROPERTY(Category = "Graph Node", Config, EditAnywhere)
	FLinearColor BorderHoveredBackgroundColor = FLinearColor(0.380392f, 0.380392f, 0.380392f, 1.0f); // gray

	/** The amount of blank space left around the edges of the speaker text area in case of speech sequence nodes. */
	UPROPERTY(Category = "Graph Node Speech Sequence", Config, EditAnywhere)
	FMargin DescriptionSpeakerMargin = FMargin(1.f);

	/** The horizontal alignment of the Speaker in case the node is a speech sequence. */
	UPROPERTY(Category = "Graph Node Speech Sequence", Config, EditAnywhere)
	TEnumAsByte<EHorizontalAlignment> DescriptionSpeakerHorizontalAlignment = HAlign_Center;

	/** The wire thickness of the connections between nodes. */
	UPROPERTY(Category = "Graph Edge", Config, EditAnywhere)
	float WireThickness = 2.0f;

	/** Flag indicating to draw the bubbles of the wire or not. */
	UPROPERTY(Category = "Graph Edge", Config, EditAnywhere)
	bool bWireDrawBubbles = false;

	/** To show or not the condition icon in the case the edge has any conditions defined on it. */
	UPROPERTY(Category = "Graph Edge", Config, EditAnywhere)
	bool bShowEdgeHasConditionsIcon = true;

	/** The base color of the wire. */
	UPROPERTY(Category = "Graph Edge", Config, EditAnywhere)
	FLinearColor WireBaseColor = FLinearColor{1.0f, 1.0f, 1.0f, 1.0f}; // white

	/** Does the wire use the condition color (if it has conditions) in the normal mode? */
	UPROPERTY(Category = "Graph Edge", Config, EditAnywhere)
	bool bShowDifferentColorForConditionWires = true;

	/** The color of the wire if the edge has any conditions. */
	UPROPERTY(Category = "Graph Edge", Config, EditAnywhere)
	FLinearColor WireWithConditionsColor = FLinearColor{1.f, 0.341176f, 0.133333f, 1.0f}; // orangeish

	/** The color of the wire when hovered over */
	UPROPERTY(Category = "Graph Edge", Config, EditAnywhere)
	FLinearColor WireHoveredColor = FLinearColor{1.0f, 0.596078f, 0.0f, 1.0f}; // orange

	/** Is the the viewing of primary/secondary edges enabled? */
	UPROPERTY(Category = "Graph Edge", Config, EditAnywhere)
	bool bShowPrimarySecondaryEdges = true;

	/** Should we draw the primary edges? */
	UPROPERTY(Category = "Graph Edge", Config, EditAnywhere)
	bool bDrawPrimaryEdges = true;

	/** Should we draw the secondary edges? */
	UPROPERTY(Category = "Graph Edge", Config, EditAnywhere)
	bool bDrawSecondaryEdges = true;

	/** The Color of the wire when the edge is primary. */
	UPROPERTY(Category = "Graph Edge", Config, EditAnywhere)
	FLinearColor WirePrimaryEdgeColor = FLinearColor{0.717647f, 0.109804f, 0.109804f, 1.0f}; // redish

	 /** The Color of the wire when the edge is secondary. */
	UPROPERTY(Category = "Graph Edge", Config, EditAnywhere)
	FLinearColor WireSecondaryEdgeColor = FLinearColor{0.101961f, 0.137255f, 0.494118f, 1.f}; // blueish

	/** Should we hide the categories in the Dialogue browser that do not have any children? */
	UPROPERTY(Category = "Browser", Config, EditAnywhere)
	bool bHideEmptyDialogueBrowserCategories = true;

	// Advanced Section
	/**  The offset on the X axis (left/right) to use when automatically positioning nodes. */
	UPROPERTY(Category = "Position", Config, EditAnywhere, AdvancedDisplay)
	int32 OffsetBetweenColumnsX = 500;

	/**  The offset on the Y axis (up/down) to use when automatically positioning nodes. */
	UPROPERTY(Category = "Position", Config, EditAnywhere, AdvancedDisplay)
	int32 OffsetBetweenRowsY = 200;
};
