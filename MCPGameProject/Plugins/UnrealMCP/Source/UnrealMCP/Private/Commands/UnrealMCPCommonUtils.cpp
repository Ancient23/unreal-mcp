#include "Commands/UnrealMCPCommonUtils.h"
#include "GameFramework/Actor.h"
#include "Engine/Blueprint.h"
#include "WidgetBlueprint.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"
#include "K2Node_Event.h"
#include "K2Node_CallFunction.h"
#include "K2Node_VariableGet.h"
#include "K2Node_VariableSet.h"
#include "K2Node_InputAction.h"
#include "K2Node_Self.h"
#include "EdGraphSchema_K2.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Components/StaticMeshComponent.h"
#include "Components/LightComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "UObject/UObjectIterator.h"
#include "Engine/Selection.h"
#include "EditorAssetLibrary.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "BlueprintNodeSpawner.h"
#include "BlueprintActionDatabase.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "EngineUtils.h"
#include "JsonObjectConverter.h"
#include "UObject/EnumProperty.h"
#include "UObject/TextProperty.h"
#include "UObject/StructOnScope.h"

// JSON Utilities
TSharedPtr<FJsonObject> FUnrealMCPCommonUtils::CreateErrorResponse(const FString& Message)
{
    TSharedPtr<FJsonObject> ResponseObject = MakeShared<FJsonObject>();
    ResponseObject->SetBoolField(TEXT("success"), false);
    ResponseObject->SetStringField(TEXT("error"), Message);
    return ResponseObject;
}

TSharedPtr<FJsonObject> FUnrealMCPCommonUtils::CreateSuccessResponse(const FString& Message /* = TEXT("") */)
{
    TSharedPtr<FJsonObject> ResponseObject = MakeShared<FJsonObject>();
    ResponseObject->SetBoolField(TEXT("success"), true);
    
    if (!Message.IsEmpty())
    {
        ResponseObject->SetStringField(TEXT("message"), Message);
    }
    
    return ResponseObject;
}

void FUnrealMCPCommonUtils::GetIntArrayFromJson(const TSharedPtr<FJsonObject>& JsonObject, const FString& FieldName, TArray<int32>& OutArray)
{
    OutArray.Reset();
    
    if (!JsonObject->HasField(FieldName))
    {
        return;
    }
    
    const TArray<TSharedPtr<FJsonValue>>* JsonArray;
    if (JsonObject->TryGetArrayField(FieldName, JsonArray))
    {
        for (const TSharedPtr<FJsonValue>& Value : *JsonArray)
        {
            OutArray.Add((int32)Value->AsNumber());
        }
    }
}

void FUnrealMCPCommonUtils::GetFloatArrayFromJson(const TSharedPtr<FJsonObject>& JsonObject, const FString& FieldName, TArray<float>& OutArray)
{
    OutArray.Reset();
    
    if (!JsonObject->HasField(FieldName))
    {
        return;
    }
    
    const TArray<TSharedPtr<FJsonValue>>* JsonArray;
    if (JsonObject->TryGetArrayField(FieldName, JsonArray))
    {
        for (const TSharedPtr<FJsonValue>& Value : *JsonArray)
        {
            OutArray.Add((float)Value->AsNumber());
        }
    }
}

FVector2D FUnrealMCPCommonUtils::GetVector2DFromJson(const TSharedPtr<FJsonObject>& JsonObject, const FString& FieldName)
{
    FVector2D Result(0.0f, 0.0f);
    
    if (!JsonObject->HasField(FieldName))
    {
        return Result;
    }
    
    const TArray<TSharedPtr<FJsonValue>>* JsonArray;
    if (JsonObject->TryGetArrayField(FieldName, JsonArray) && JsonArray->Num() >= 2)
    {
        Result.X = (float)(*JsonArray)[0]->AsNumber();
        Result.Y = (float)(*JsonArray)[1]->AsNumber();
    }
    
    return Result;
}

FVector FUnrealMCPCommonUtils::GetVectorFromJson(const TSharedPtr<FJsonObject>& JsonObject, const FString& FieldName)
{
    FVector Result(0.0f, 0.0f, 0.0f);
    
    if (!JsonObject->HasField(FieldName))
    {
        return Result;
    }
    
    const TArray<TSharedPtr<FJsonValue>>* JsonArray;
    if (JsonObject->TryGetArrayField(FieldName, JsonArray) && JsonArray->Num() >= 3)
    {
        Result.X = (float)(*JsonArray)[0]->AsNumber();
        Result.Y = (float)(*JsonArray)[1]->AsNumber();
        Result.Z = (float)(*JsonArray)[2]->AsNumber();
    }
    
    return Result;
}

FRotator FUnrealMCPCommonUtils::GetRotatorFromJson(const TSharedPtr<FJsonObject>& JsonObject, const FString& FieldName)
{
    FRotator Result(0.0f, 0.0f, 0.0f);
    
    if (!JsonObject->HasField(FieldName))
    {
        return Result;
    }
    
    const TArray<TSharedPtr<FJsonValue>>* JsonArray;
    if (JsonObject->TryGetArrayField(FieldName, JsonArray) && JsonArray->Num() >= 3)
    {
        Result.Pitch = (float)(*JsonArray)[0]->AsNumber();
        Result.Yaw = (float)(*JsonArray)[1]->AsNumber();
        Result.Roll = (float)(*JsonArray)[2]->AsNumber();
    }
    
    return Result;
}

// Blueprint Utilities
UBlueprint* FUnrealMCPCommonUtils::FindBlueprint(const FString& BlueprintName)
{
    return FindBlueprintByName(BlueprintName);
}

UBlueprint* FUnrealMCPCommonUtils::FindBlueprintByName(const FString& BlueprintName)
{
    // Early exit for empty names
    if (BlueprintName.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Empty blueprint name provided"));
        return nullptr;
    }

    // Step 1: Normalize the path
    FString NormalizedName = BlueprintName;
    
    // Remove .uasset extension if present
    if (NormalizedName.EndsWith(TEXT(".uasset"), ESearchCase::IgnoreCase))
    {
        NormalizedName = NormalizedName.LeftChop(7);
    }
    
    // Handle absolute vs relative paths
    bool bIsAbsolutePath = NormalizedName.StartsWith(TEXT("/"));
    if (bIsAbsolutePath)
    {
        // If it's an absolute path starting with /Game/, use it directly
        if (NormalizedName.StartsWith(TEXT("/Game/")))
        {
            UE_LOG(LogTemp, Display, TEXT("Using absolute path: %s"), *NormalizedName);
            UBlueprint* Blueprint = LoadObject<UBlueprint>(nullptr, *NormalizedName);
            if (Blueprint)
            {
                return Blueprint;
            }
        }
        // If it starts with / but not /Game/, prepend /Game/
        else
        {
            NormalizedName = FString(TEXT("/Game")) + NormalizedName;
            UE_LOG(LogTemp, Display, TEXT("Converted to game path: %s"), *NormalizedName);
            UBlueprint* Blueprint = LoadObject<UBlueprint>(nullptr, *NormalizedName);
            if (Blueprint)
            {
                return Blueprint;
            }
        }
    }
    else
    {
        // For relative paths, extract any subdirectories
        FString SubPath;
        FString BaseName;
        if (NormalizedName.Contains(TEXT("/")))
        {
            NormalizedName.Split(TEXT("/"), &SubPath, &BaseName, ESearchCase::CaseSensitive, ESearchDir::FromEnd);
            // Reconstruct with /Game/ prefix
            NormalizedName = FString::Printf(TEXT("/Game/%s/%s"), *SubPath, *BaseName);
            UE_LOG(LogTemp, Display, TEXT("Reconstructed path with subdirectory: %s"), *NormalizedName);
            UBlueprint* Blueprint = LoadObject<UBlueprint>(nullptr, *NormalizedName);
            if (Blueprint)
            {
                return Blueprint;
            }
        }
        else
        {
            BaseName = NormalizedName;
        }

        // Try standard locations for relative paths
        TArray<FString> DefaultPaths = {
            FString::Printf(TEXT("/Game/Blueprints/%s"), *BaseName),
            FString::Printf(TEXT("/Game/%s"), *BaseName)
        };

        // Try each default path
        for (const FString& Path : DefaultPaths)
        {
            UE_LOG(LogTemp, Display, TEXT("Trying blueprint at path: %s"), *Path);
            UBlueprint* Blueprint = LoadObject<UBlueprint>(nullptr, *Path);
            if (Blueprint)
            {
                return Blueprint;
            }
        }
    }

    // If still not found, use asset registry for a thorough search
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    TArray<FAssetData> AllBlueprintAssetData;
    
    // Create a filter for blueprints and widget blueprints
    FARFilter Filter;
    Filter.ClassPaths.Add(UBlueprint::StaticClass()->GetClassPathName());
    Filter.ClassPaths.Add(UWidgetBlueprint::StaticClass()->GetClassPathName());
    Filter.PackagePaths.Add(TEXT("/Game"));
    Filter.bRecursivePaths = true;

    UE_LOG(LogTemp, Display, TEXT("Performing Asset Registry search for: %s"), *NormalizedName);
    AssetRegistryModule.Get().GetAssets(Filter, AllBlueprintAssetData);
    UE_LOG(LogTemp, Display, TEXT("Found %d total blueprint assets"), AllBlueprintAssetData.Num());

    // First try exact name match
    FString SearchName = FPaths::GetBaseFilename(NormalizedName);
    for (const FAssetData& Asset : AllBlueprintAssetData)
    {
        if (Asset.AssetName.ToString() == SearchName)
        {
            UE_LOG(LogTemp, Display, TEXT("Found exact match: %s"), *Asset.GetObjectPathString());
            return Cast<UBlueprint>(Asset.GetAsset());
        }
    }

    // If exact match fails, try case-insensitive match
    for (const FAssetData& Asset : AllBlueprintAssetData)
    {
        if (Asset.AssetName.ToString().Equals(SearchName, ESearchCase::IgnoreCase))
        {
            UE_LOG(LogTemp, Warning, TEXT("Found case-insensitive match: %s"), *Asset.GetObjectPathString());
            return Cast<UBlueprint>(Asset.GetAsset());
        }
    }

    UE_LOG(LogTemp, Error, TEXT("Blueprint '%s' not found after exhaustive search"), *BlueprintName);
    return nullptr;
}

UEdGraph* FUnrealMCPCommonUtils::FindOrCreateEventGraph(UBlueprint* Blueprint)
{
    if (!Blueprint)
    {
        return nullptr;
    }
    
    // Try to find the event graph
    for (UEdGraph* Graph : Blueprint->UbergraphPages)
    {
        if (Graph->GetName().Contains(TEXT("EventGraph")))
        {
            return Graph;
        }
    }
    
    // Create a new event graph if none exists
    UEdGraph* NewGraph = FBlueprintEditorUtils::CreateNewGraph(Blueprint, FName(TEXT("EventGraph")), UEdGraph::StaticClass(), UEdGraphSchema_K2::StaticClass());
    FBlueprintEditorUtils::AddUbergraphPage(Blueprint, NewGraph);
    return NewGraph;
}

// Blueprint node utilities
UK2Node_Event* FUnrealMCPCommonUtils::CreateEventNode(UEdGraph* Graph, const FString& EventName, const FVector2D& Position)
{
    if (!Graph)
    {
        return nullptr;
    }
    
    UBlueprint* Blueprint = FBlueprintEditorUtils::FindBlueprintForGraph(Graph);
    if (!Blueprint)
    {
        return nullptr;
    }
    
    // Check for existing event node with this exact name
    for (UEdGraphNode* Node : Graph->Nodes)
    {
        UK2Node_Event* EventNode = Cast<UK2Node_Event>(Node);
        if (EventNode && EventNode->EventReference.GetMemberName() == FName(*EventName))
        {
            UE_LOG(LogTemp, Display, TEXT("Using existing event node with name %s (ID: %s)"), 
                *EventName, *EventNode->NodeGuid.ToString());
            return EventNode;
        }
    }

    // No existing node found, create a new one
    UK2Node_Event* EventNode = nullptr;
    
    // Find the function to create the event
    UClass* BlueprintClass = Blueprint->GeneratedClass;
    UFunction* EventFunction = BlueprintClass->FindFunctionByName(FName(*EventName));
    
    if (EventFunction)
    {
        EventNode = NewObject<UK2Node_Event>(Graph);
        EventNode->EventReference.SetExternalMember(FName(*EventName), BlueprintClass);
        EventNode->NodePosX = Position.X;
        EventNode->NodePosY = Position.Y;
        Graph->AddNode(EventNode, true);
        EventNode->PostPlacedNewNode();
        EventNode->AllocateDefaultPins();
        UE_LOG(LogTemp, Display, TEXT("Created new event node with name %s (ID: %s)"), 
            *EventName, *EventNode->NodeGuid.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to find function for event name: %s"), *EventName);
    }
    
    return EventNode;
}

UK2Node_CallFunction* FUnrealMCPCommonUtils::CreateFunctionCallNode(UEdGraph* Graph, UFunction* Function, const FVector2D& Position)
{
    if (!Graph || !Function)
    {
        return nullptr;
    }
    
    UK2Node_CallFunction* FunctionNode = NewObject<UK2Node_CallFunction>(Graph);
    FunctionNode->SetFromFunction(Function);
    FunctionNode->NodePosX = Position.X;
    FunctionNode->NodePosY = Position.Y;
    Graph->AddNode(FunctionNode, true);
    FunctionNode->CreateNewGuid();
    FunctionNode->PostPlacedNewNode();
    FunctionNode->AllocateDefaultPins();
    
    return FunctionNode;
}

UK2Node_VariableGet* FUnrealMCPCommonUtils::CreateVariableGetNode(UEdGraph* Graph, UBlueprint* Blueprint, const FString& VariableName, const FVector2D& Position)
{
    if (!Graph || !Blueprint)
    {
        return nullptr;
    }
    
    UK2Node_VariableGet* VariableGetNode = NewObject<UK2Node_VariableGet>(Graph);
    
    FName VarName(*VariableName);
    FProperty* Property = FindFProperty<FProperty>(Blueprint->GeneratedClass, VarName);
    
    if (Property)
    {
        VariableGetNode->VariableReference.SetFromField<FProperty>(Property, false);
        VariableGetNode->NodePosX = Position.X;
        VariableGetNode->NodePosY = Position.Y;
        Graph->AddNode(VariableGetNode, true);
        VariableGetNode->PostPlacedNewNode();
        VariableGetNode->AllocateDefaultPins();
        
        return VariableGetNode;
    }
    
    return nullptr;
}

UK2Node_VariableSet* FUnrealMCPCommonUtils::CreateVariableSetNode(UEdGraph* Graph, UBlueprint* Blueprint, const FString& VariableName, const FVector2D& Position)
{
    if (!Graph || !Blueprint)
    {
        return nullptr;
    }
    
    UK2Node_VariableSet* VariableSetNode = NewObject<UK2Node_VariableSet>(Graph);
    
    FName VarName(*VariableName);
    FProperty* Property = FindFProperty<FProperty>(Blueprint->GeneratedClass, VarName);
    
    if (Property)
    {
        VariableSetNode->VariableReference.SetFromField<FProperty>(Property, false);
        VariableSetNode->NodePosX = Position.X;
        VariableSetNode->NodePosY = Position.Y;
        Graph->AddNode(VariableSetNode, true);
        VariableSetNode->PostPlacedNewNode();
        VariableSetNode->AllocateDefaultPins();
        
        return VariableSetNode;
    }
    
    return nullptr;
}

UK2Node_InputAction* FUnrealMCPCommonUtils::CreateInputActionNode(UEdGraph* Graph, const FString& ActionName, const FVector2D& Position)
{
    if (!Graph)
    {
        return nullptr;
    }
    
    UK2Node_InputAction* InputActionNode = NewObject<UK2Node_InputAction>(Graph);
    InputActionNode->InputActionName = FName(*ActionName);
    InputActionNode->NodePosX = Position.X;
    InputActionNode->NodePosY = Position.Y;
    Graph->AddNode(InputActionNode, true);
    InputActionNode->CreateNewGuid();
    InputActionNode->PostPlacedNewNode();
    InputActionNode->AllocateDefaultPins();
    
    return InputActionNode;
}

UK2Node_Self* FUnrealMCPCommonUtils::CreateSelfReferenceNode(UEdGraph* Graph, const FVector2D& Position)
{
    if (!Graph)
    {
        return nullptr;
    }
    
    UK2Node_Self* SelfNode = NewObject<UK2Node_Self>(Graph);
    SelfNode->NodePosX = Position.X;
    SelfNode->NodePosY = Position.Y;
    Graph->AddNode(SelfNode, true);
    SelfNode->CreateNewGuid();
    SelfNode->PostPlacedNewNode();
    SelfNode->AllocateDefaultPins();
    
    return SelfNode;
}

bool FUnrealMCPCommonUtils::ConnectGraphNodes(UEdGraph* Graph, UEdGraphNode* SourceNode, const FString& SourcePinName, 
                                           UEdGraphNode* TargetNode, const FString& TargetPinName)
{
    if (!Graph || !SourceNode || !TargetNode)
    {
        return false;
    }
    
    UEdGraphPin* SourcePin = FindPin(SourceNode, SourcePinName, EGPD_Output);
    UEdGraphPin* TargetPin = FindPin(TargetNode, TargetPinName, EGPD_Input);
    
    if (SourcePin && TargetPin)
    {
        SourcePin->MakeLinkTo(TargetPin);
        return true;
    }
    
    return false;
}

UEdGraphPin* FUnrealMCPCommonUtils::FindPin(UEdGraphNode* Node, const FString& PinName, EEdGraphPinDirection Direction)
{
    if (!Node)
    {
        return nullptr;
    }
    
    // Log all pins for debugging
    UE_LOG(LogTemp, Display, TEXT("FindPin: Looking for pin '%s' (Direction: %d) in node '%s'"), 
           *PinName, (int32)Direction, *Node->GetName());
    
    for (UEdGraphPin* Pin : Node->Pins)
    {
        UE_LOG(LogTemp, Display, TEXT("  - Available pin: '%s', Direction: %d, Category: %s"), 
               *Pin->PinName.ToString(), (int32)Pin->Direction, *Pin->PinType.PinCategory.ToString());
    }
    
    // First try exact match
    for (UEdGraphPin* Pin : Node->Pins)
    {
        if (Pin->PinName.ToString() == PinName && (Direction == EGPD_MAX || Pin->Direction == Direction))
        {
            UE_LOG(LogTemp, Display, TEXT("  - Found exact matching pin: '%s'"), *Pin->PinName.ToString());
            return Pin;
        }
    }
    
    // If no exact match and we're looking for a component reference, try case-insensitive match
    for (UEdGraphPin* Pin : Node->Pins)
    {
        if (Pin->PinName.ToString().Equals(PinName, ESearchCase::IgnoreCase) && 
            (Direction == EGPD_MAX || Pin->Direction == Direction))
        {
            UE_LOG(LogTemp, Display, TEXT("  - Found case-insensitive matching pin: '%s'"), *Pin->PinName.ToString());
            return Pin;
        }
    }
    
    // If we're looking for a component output and didn't find it by name, try to find the first data output pin
    if (Direction == EGPD_Output && Cast<UK2Node_VariableGet>(Node) != nullptr)
    {
        for (UEdGraphPin* Pin : Node->Pins)
        {
            if (Pin->Direction == EGPD_Output && Pin->PinType.PinCategory != UEdGraphSchema_K2::PC_Exec)
            {
                UE_LOG(LogTemp, Display, TEXT("  - Found fallback data output pin: '%s'"), *Pin->PinName.ToString());
                return Pin;
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("  - No matching pin found for '%s'"), *PinName);
    return nullptr;
}

// Actor utilities
TSharedPtr<FJsonValue> FUnrealMCPCommonUtils::ActorToJson(AActor* Actor)
{
    if (!Actor)
    {
        return MakeShared<FJsonValueNull>();
    }
    
    TSharedPtr<FJsonObject> ActorObject = MakeShared<FJsonObject>();
    ActorObject->SetStringField(TEXT("name"), Actor->GetName());
    ActorObject->SetStringField(TEXT("class"), Actor->GetClass()->GetName());
    
    FVector Location = Actor->GetActorLocation();
    TArray<TSharedPtr<FJsonValue>> LocationArray;
    LocationArray.Add(MakeShared<FJsonValueNumber>(Location.X));
    LocationArray.Add(MakeShared<FJsonValueNumber>(Location.Y));
    LocationArray.Add(MakeShared<FJsonValueNumber>(Location.Z));
    ActorObject->SetArrayField(TEXT("location"), LocationArray);
    
    FRotator Rotation = Actor->GetActorRotation();
    TArray<TSharedPtr<FJsonValue>> RotationArray;
    RotationArray.Add(MakeShared<FJsonValueNumber>(Rotation.Pitch));
    RotationArray.Add(MakeShared<FJsonValueNumber>(Rotation.Yaw));
    RotationArray.Add(MakeShared<FJsonValueNumber>(Rotation.Roll));
    ActorObject->SetArrayField(TEXT("rotation"), RotationArray);
    
    FVector Scale = Actor->GetActorScale3D();
    TArray<TSharedPtr<FJsonValue>> ScaleArray;
    ScaleArray.Add(MakeShared<FJsonValueNumber>(Scale.X));
    ScaleArray.Add(MakeShared<FJsonValueNumber>(Scale.Y));
    ScaleArray.Add(MakeShared<FJsonValueNumber>(Scale.Z));
    ActorObject->SetArrayField(TEXT("scale"), ScaleArray);
    
    return MakeShared<FJsonValueObject>(ActorObject);
}

TSharedPtr<FJsonObject> FUnrealMCPCommonUtils::ActorToJsonObject(AActor* Actor, bool bDetailed)
{
    if (!Actor)
    {
        return nullptr;
    }
    
    TSharedPtr<FJsonObject> ActorObject = MakeShared<FJsonObject>();
    ActorObject->SetStringField(TEXT("name"), Actor->GetName());
    ActorObject->SetStringField(TEXT("class"), Actor->GetClass()->GetName());
    
    FVector Location = Actor->GetActorLocation();
    TArray<TSharedPtr<FJsonValue>> LocationArray;
    LocationArray.Add(MakeShared<FJsonValueNumber>(Location.X));
    LocationArray.Add(MakeShared<FJsonValueNumber>(Location.Y));
    LocationArray.Add(MakeShared<FJsonValueNumber>(Location.Z));
    ActorObject->SetArrayField(TEXT("location"), LocationArray);
    
    FRotator Rotation = Actor->GetActorRotation();
    TArray<TSharedPtr<FJsonValue>> RotationArray;
    RotationArray.Add(MakeShared<FJsonValueNumber>(Rotation.Pitch));
    RotationArray.Add(MakeShared<FJsonValueNumber>(Rotation.Yaw));
    RotationArray.Add(MakeShared<FJsonValueNumber>(Rotation.Roll));
    ActorObject->SetArrayField(TEXT("rotation"), RotationArray);
    
    FVector Scale = Actor->GetActorScale3D();
    TArray<TSharedPtr<FJsonValue>> ScaleArray;
    ScaleArray.Add(MakeShared<FJsonValueNumber>(Scale.X));
    ScaleArray.Add(MakeShared<FJsonValueNumber>(Scale.Y));
    ScaleArray.Add(MakeShared<FJsonValueNumber>(Scale.Z));
    ActorObject->SetArrayField(TEXT("scale"), ScaleArray);
    
    return ActorObject;
}

UK2Node_Event* FUnrealMCPCommonUtils::FindExistingEventNode(UEdGraph* Graph, const FString& EventName)
{
    if (!Graph)
    {
        return nullptr;
    }

    // Look for existing event nodes
    for (UEdGraphNode* Node : Graph->Nodes)
    {
        UK2Node_Event* EventNode = Cast<UK2Node_Event>(Node);
        if (EventNode && EventNode->EventReference.GetMemberName() == FName(*EventName))
        {
            UE_LOG(LogTemp, Display, TEXT("Found existing event node with name: %s"), *EventName);
            return EventNode;
        }
    }

    return nullptr;
}

bool FUnrealMCPCommonUtils::SetObjectProperty(UObject* Object, const FString& PropertyName, 
                                     const TSharedPtr<FJsonValue>& Value, FString& OutErrorMessage)
{
    if (!Object)
    {
        OutErrorMessage = TEXT("Invalid object");
        return false;
    }

    FProperty* Property = Object->GetClass()->FindPropertyByName(*PropertyName);
    if (!Property)
    {
        OutErrorMessage = FString::Printf(TEXT("Property not found: %s"), *PropertyName);
        return false;
    }

    void* PropertyAddr = Property->ContainerPtrToValuePtr<void>(Object);
    
    // Handle different property types
    if (Property->IsA<FBoolProperty>())
    {
        ((FBoolProperty*)Property)->SetPropertyValue(PropertyAddr, Value->AsBool());
        return true;
    }
    else if (Property->IsA<FIntProperty>())
    {
        int32 IntValue = static_cast<int32>(Value->AsNumber());
        FIntProperty* IntProperty = CastField<FIntProperty>(Property);
        if (IntProperty)
        {
            IntProperty->SetPropertyValue_InContainer(Object, IntValue);
            return true;
        }
    }
    else if (Property->IsA<FFloatProperty>())
    {
        ((FFloatProperty*)Property)->SetPropertyValue(PropertyAddr, Value->AsNumber());
        return true;
    }
    else if (Property->IsA<FStrProperty>())
    {
        ((FStrProperty*)Property)->SetPropertyValue(PropertyAddr, Value->AsString());
        return true;
    }
    else if (Property->IsA<FByteProperty>())
    {
        FByteProperty* ByteProp = CastField<FByteProperty>(Property);
        UEnum* EnumDef = ByteProp ? ByteProp->GetIntPropertyEnum() : nullptr;
        
        // If this is a TEnumAsByte property (has associated enum)
        if (EnumDef)
        {
            // Handle numeric value
            if (Value->Type == EJson::Number)
            {
                uint8 ByteValue = static_cast<uint8>(Value->AsNumber());
                ByteProp->SetPropertyValue(PropertyAddr, ByteValue);
                
                UE_LOG(LogTemp, Display, TEXT("Setting enum property %s to numeric value: %d"), 
                      *PropertyName, ByteValue);
                return true;
            }
            // Handle string enum value
            else if (Value->Type == EJson::String)
            {
                FString EnumValueName = Value->AsString();
                
                // Try to convert numeric string to number first
                if (EnumValueName.IsNumeric())
                {
                    uint8 ByteValue = FCString::Atoi(*EnumValueName);
                    ByteProp->SetPropertyValue(PropertyAddr, ByteValue);
                    
                    UE_LOG(LogTemp, Display, TEXT("Setting enum property %s to numeric string value: %s -> %d"), 
                          *PropertyName, *EnumValueName, ByteValue);
                    return true;
                }
                
                // Handle qualified enum names (e.g., "Player0" or "EAutoReceiveInput::Player0")
                if (EnumValueName.Contains(TEXT("::")))
                {
                    EnumValueName.Split(TEXT("::"), nullptr, &EnumValueName);
                }
                
                int64 EnumValue = EnumDef->GetValueByNameString(EnumValueName);
                if (EnumValue == INDEX_NONE)
                {
                    // Try with full name as fallback
                    EnumValue = EnumDef->GetValueByNameString(Value->AsString());
                }
                
                if (EnumValue != INDEX_NONE)
                {
                    ByteProp->SetPropertyValue(PropertyAddr, static_cast<uint8>(EnumValue));
                    
                    UE_LOG(LogTemp, Display, TEXT("Setting enum property %s to name value: %s -> %lld"), 
                          *PropertyName, *EnumValueName, EnumValue);
                    return true;
                }
                else
                {
                    // Log all possible enum values for debugging
                    UE_LOG(LogTemp, Warning, TEXT("Could not find enum value for '%s'. Available options:"), *EnumValueName);
                    for (int32 i = 0; i < EnumDef->NumEnums(); i++)
                    {
                        UE_LOG(LogTemp, Warning, TEXT("  - %s (value: %d)"), 
                               *EnumDef->GetNameStringByIndex(i), EnumDef->GetValueByIndex(i));
                    }
                    
                    OutErrorMessage = FString::Printf(TEXT("Could not find enum value for '%s'"), *EnumValueName);
                    return false;
                }
            }
        }
        else
        {
            // Regular byte property
            uint8 ByteValue = static_cast<uint8>(Value->AsNumber());
            ByteProp->SetPropertyValue(PropertyAddr, ByteValue);
            return true;
        }
    }
    else if (Property->IsA<FEnumProperty>())
    {
        FEnumProperty* EnumProp = CastField<FEnumProperty>(Property);
        UEnum* EnumDef = EnumProp ? EnumProp->GetEnum() : nullptr;
        FNumericProperty* UnderlyingNumericProp = EnumProp ? EnumProp->GetUnderlyingProperty() : nullptr;
        
        if (EnumDef && UnderlyingNumericProp)
        {
            // Handle numeric value
            if (Value->Type == EJson::Number)
            {
                int64 EnumValue = static_cast<int64>(Value->AsNumber());
                UnderlyingNumericProp->SetIntPropertyValue(PropertyAddr, EnumValue);
                
                UE_LOG(LogTemp, Display, TEXT("Setting enum property %s to numeric value: %lld"), 
                      *PropertyName, EnumValue);
                return true;
            }
            // Handle string enum value
            else if (Value->Type == EJson::String)
            {
                FString EnumValueName = Value->AsString();
                
                // Try to convert numeric string to number first
                if (EnumValueName.IsNumeric())
                {
                    int64 EnumValue = FCString::Atoi64(*EnumValueName);
                    UnderlyingNumericProp->SetIntPropertyValue(PropertyAddr, EnumValue);
                    
                    UE_LOG(LogTemp, Display, TEXT("Setting enum property %s to numeric string value: %s -> %lld"), 
                          *PropertyName, *EnumValueName, EnumValue);
                    return true;
                }
                
                // Handle qualified enum names
                if (EnumValueName.Contains(TEXT("::")))
                {
                    EnumValueName.Split(TEXT("::"), nullptr, &EnumValueName);
                }
                
                int64 EnumValue = EnumDef->GetValueByNameString(EnumValueName);
                if (EnumValue == INDEX_NONE)
                {
                    // Try with full name as fallback
                    EnumValue = EnumDef->GetValueByNameString(Value->AsString());
                }
                
                if (EnumValue != INDEX_NONE)
                {
                    UnderlyingNumericProp->SetIntPropertyValue(PropertyAddr, EnumValue);
                    
                    UE_LOG(LogTemp, Display, TEXT("Setting enum property %s to name value: %s -> %lld"), 
                          *PropertyName, *EnumValueName, EnumValue);
                    return true;
                }
                else
                {
                    // Log all possible enum values for debugging
                    UE_LOG(LogTemp, Warning, TEXT("Could not find enum value for '%s'. Available options:"), *EnumValueName);
                    for (int32 i = 0; i < EnumDef->NumEnums(); i++)
                    {
                        UE_LOG(LogTemp, Warning, TEXT("  - %s (value: %d)"), 
                               *EnumDef->GetNameStringByIndex(i), EnumDef->GetValueByIndex(i));
                    }
                    
                    OutErrorMessage = FString::Printf(TEXT("Could not find enum value for '%s'"), *EnumValueName);
                    return false;
                }
            }
        }
    }
    else if (FStructProperty* StructProp = CastField<FStructProperty>(Property))
    {
        if (Value->Type == EJson::Array)
        {
            const TArray<TSharedPtr<FJsonValue>>& Arr = Value->AsArray();
            bool bStructHandled = false;

            // Handle FVector2D
            if (StructProp->Struct == TBaseStructure<FVector2D>::Get())
            {
                if (Arr.Num() == 2)
                {
                    FVector2D Vec2D(Arr[0]->AsNumber(), Arr[1]->AsNumber());
                    StructProp->CopySingleValue(PropertyAddr, &Vec2D);
                    UE_LOG(LogTemp, Display, TEXT("Setting FVector2D property %s to (%f, %f)"), 
                          *PropertyName, Vec2D.X, Vec2D.Y);
                    bStructHandled = true;
                }
                else
                {
                    OutErrorMessage = FString::Printf(TEXT("FVector2D property requires 2 values, got %d"), Arr.Num());
                }
            }
            // Handle FLinearColor
            else if (StructProp->Struct == TBaseStructure<FLinearColor>::Get())
            {
                if (Arr.Num() == 4) // RGBA
                {
                    FLinearColor Color(
                        Arr[0]->AsNumber(),
                        Arr[1]->AsNumber(),
                        Arr[2]->AsNumber(),
                        Arr[3]->AsNumber()
                    );
                    StructProp->CopySingleValue(PropertyAddr, &Color);
                    UE_LOG(LogTemp, Display, TEXT("Setting FLinearColor property %s to (R=%f, G=%f, B=%f, A=%f)"), 
                          *PropertyName, Color.R, Color.G, Color.B, Color.A);
                    bStructHandled = true;
                }
                 else if (Arr.Num() == 3) // RGB, assume A=1
                {
                    FLinearColor Color(
                        Arr[0]->AsNumber(),
                        Arr[1]->AsNumber(),
                        Arr[2]->AsNumber(),
                        1.0f // Default Alpha to 1
                    );
                    StructProp->CopySingleValue(PropertyAddr, &Color);
                    UE_LOG(LogTemp, Display, TEXT("Setting FLinearColor property %s to (R=%f, G=%f, B=%f, A=1.0)"), 
                          *PropertyName, Color.R, Color.G, Color.B);
                    bStructHandled = true;
                }
                else
                {
                    OutErrorMessage = FString::Printf(TEXT("FLinearColor property requires 3 (RGB) or 4 (RGBA) values, got %d"), Arr.Num());
                }
            }
            // Handle FRotator
            else if (StructProp->Struct == TBaseStructure<FRotator>::Get())
            {
                if (Arr.Num() == 3) // Pitch, Yaw, Roll
                {
                    FRotator Rotator(
                        Arr[0]->AsNumber(), // Pitch
                        Arr[1]->AsNumber(), // Yaw
                        Arr[2]->AsNumber()  // Roll
                    );
                    StructProp->CopySingleValue(PropertyAddr, &Rotator);
                    UE_LOG(LogTemp, Display, TEXT("Setting FRotator property %s to (P=%f, Y=%f, R=%f)"), 
                          *PropertyName, Rotator.Pitch, Rotator.Yaw, Rotator.Roll);
                    bStructHandled = true;
                }
                else
                {
                    OutErrorMessage = FString::Printf(TEXT("FRotator property requires 3 values (Pitch, Yaw, Roll), got %d"), Arr.Num());
                }
            }
            // NOTE: FVector is handled specifically in HandleSetComponentProperty currently, 
            // but could be moved here for consistency if desired.
            
            if (bStructHandled)
            {
                return true; // Successfully handled the struct
            }
        }
        else
        {
             OutErrorMessage = FString::Printf(TEXT("Struct property %s requires a JSON array value"), *PropertyName);
        }
        // If we reach here, the struct type wasn't handled or input was wrong
        if (OutErrorMessage.IsEmpty())
        {
            OutErrorMessage = FString::Printf(TEXT("Unsupported struct type '%s' for property %s"), 
                StructProp->Struct ? *StructProp->Struct->GetName() : TEXT("Unknown"), *PropertyName);
        }
        return false;
    }
    
    OutErrorMessage = FString::Printf(TEXT("Unsupported property type: %s for property %s"), 
                                    *Property->GetClass()->GetName(), *PropertyName);
    return false;
}

// Implementation for the new helper function
bool FUnrealMCPCommonUtils::SetPropertyFromJson(FProperty* Property, void* ContainerPtr, const TSharedPtr<FJsonValue>& JsonValue)
{
    if (!Property || !ContainerPtr || !JsonValue.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("SetPropertyFromJson: Invalid input parameter(s)."));
        return false;
    }

    UE_LOG(LogTemp, Log, TEXT("SetPropertyFromJson - Property Name: %s, Type: %s"), *Property->GetName(), *Property->GetCPPType());

    // Handle different property types
    if (FBoolProperty* BoolProperty = CastField<FBoolProperty>(Property))
    {
        bool Value;
        if (JsonValue->TryGetBool(Value))
        {
            UE_LOG(LogTemp, Log, TEXT("SetPropertyFromJson - Setting Bool property to: %s"), Value ? TEXT("true") : TEXT("false"));
            BoolProperty->SetPropertyValue(ContainerPtr, Value);
            return true;
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("SetPropertyFromJson - Failed to set Bool property, incompatible value type"));
        }
    }
    else if (FIntProperty* IntProperty = CastField<FIntProperty>(Property))
    {
        int32 Value;
        if (JsonValue->TryGetNumber(Value))
        {
            UE_LOG(LogTemp, Log, TEXT("SetPropertyFromJson - Setting Int property to: %d"), Value);
            IntProperty->SetPropertyValue(ContainerPtr, Value);
            return true;
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("SetPropertyFromJson - Failed to set Int property, incompatible value type"));
        }
    }
    else if (FFloatProperty* FloatProperty = CastField<FFloatProperty>(Property))
    {
        double Value; // JSON numbers are doubles
        if (JsonValue->TryGetNumber(Value))
        {
            UE_LOG(LogTemp, Log, TEXT("SetPropertyFromJson - Setting Float property to: %f"), static_cast<float>(Value));
            FloatProperty->SetPropertyValue(ContainerPtr, static_cast<float>(Value));
            return true;
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("SetPropertyFromJson - Failed to set Float property, incompatible value type"));
        }
    }
    else if (FDoubleProperty* DoubleProperty = CastField<FDoubleProperty>(Property))
    {
        double Value;
        if (JsonValue->TryGetNumber(Value))
        {
            DoubleProperty->SetPropertyValue(ContainerPtr, Value);
            return true;
        }
    }
    else if (FStrProperty* StrProperty = CastField<FStrProperty>(Property))
    {
        FString Value;
        if (JsonValue->TryGetString(Value))
        {
            StrProperty->SetPropertyValue(ContainerPtr, Value);
            return true;
        }
    }
    else if (FNameProperty* NameProperty = CastField<FNameProperty>(Property))
    {
        FString Value;
        if (JsonValue->TryGetString(Value))
        {
            NameProperty->SetPropertyValue(ContainerPtr, FName(*Value));
            return true;
        }
    }
    else if (FTextProperty* TextProperty = CastField<FTextProperty>(Property))
    {
        FString Value;
        if (JsonValue->TryGetString(Value))
        {
            TextProperty->SetPropertyValue(ContainerPtr, FText::FromString(Value));
            return true;
        }
    }
    else if (FEnumProperty* EnumProperty = CastField<FEnumProperty>(Property))
    {
        UEnum* Enum = EnumProperty->GetEnum();
        if (!Enum) return false;

        FString StringValue;
        int64 IntValue;

        if (JsonValue->TryGetString(StringValue)) // Try setting by string name first
        {
            IntValue = Enum->GetValueByNameString(StringValue);
            if (IntValue != INDEX_NONE)
            {
                EnumProperty->GetUnderlyingProperty()->SetIntPropertyValue(ContainerPtr, IntValue);
                return true;
            }
        }
        else if (JsonValue->TryGetNumber(IntValue)) // Try setting by integer index
        {
             if (Enum->IsValidEnumValue(IntValue))
             {
                 EnumProperty->GetUnderlyingProperty()->SetIntPropertyValue(ContainerPtr, IntValue);
                 return true;
             }
        }
    }
    else if (FStructProperty* StructProperty = CastField<FStructProperty>(Property))
    {
        UE_LOG(LogTemp, Log, TEXT("SetPropertyFromJson - Found Struct property: %s"), 
            StructProperty->Struct ? *StructProperty->Struct->GetName() : TEXT("NULL"));
            
        const TSharedPtr<FJsonObject>* JsonObject;
        if (JsonValue->TryGetObject(JsonObject))
        {
            UE_LOG(LogTemp, Log, TEXT("SetPropertyFromJson - Processing JsonObject for struct"));
            // Use JsonObjectConverter to convert the JSON object to the struct
            if (FJsonObjectConverter::JsonObjectToUStruct(JsonObject->ToSharedRef(), StructProperty->Struct, ContainerPtr, 0, 0))
            {
                UE_LOG(LogTemp, Log, TEXT("SetPropertyFromJson - Successfully converted JsonObject to struct"));
                return true;
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("SetPropertyFromJson - Failed to convert JsonObject to struct"));
            }
        }
        // Handle common structs specifically if needed (e.g., FVector, FLinearColor from array)
        else if (StructProperty->Struct == TBaseStructure<FVector>::Get()) 
        {
            UE_LOG(LogTemp, Log, TEXT("SetPropertyFromJson - Handling FVector struct"));
            const TArray<TSharedPtr<FJsonValue>>* JsonArray;
            if (JsonValue->TryGetArray(JsonArray)) 
            {
                UE_LOG(LogTemp, Log, TEXT("SetPropertyFromJson - Got array for FVector with %d elements"), JsonArray->Num());
                FVector VecValue;
                if (ParseVector(*JsonArray, VecValue))
                {
                    UE_LOG(LogTemp, Log, TEXT("SetPropertyFromJson - Setting FVector to (%f, %f, %f)"), 
                        VecValue.X, VecValue.Y, VecValue.Z);
                    *static_cast<FVector*>(ContainerPtr) = VecValue;
                    return true;
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("SetPropertyFromJson - Failed to parse Vector from array"));
                }
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("SetPropertyFromJson - Expected array for Vector but got different type"));
            }
        }
        else if (StructProperty->Struct == TBaseStructure<FLinearColor>::Get())
        {
            UE_LOG(LogTemp, Log, TEXT("SetPropertyFromJson - Handling FLinearColor struct"));
            const TArray<TSharedPtr<FJsonValue>>* JsonArray;
            if (JsonValue->TryGetArray(JsonArray))
            {
                UE_LOG(LogTemp, Log, TEXT("SetPropertyFromJson - Got array for FLinearColor with %d elements"), JsonArray->Num());
                FLinearColor ColorValue;
                if (ParseLinearColor(*JsonArray, ColorValue))
                {
                    UE_LOG(LogTemp, Log, TEXT("SetPropertyFromJson - Setting FLinearColor to (%f, %f, %f, %f)"), 
                        ColorValue.R, ColorValue.G, ColorValue.B, ColorValue.A);
                    *static_cast<FLinearColor*>(ContainerPtr) = ColorValue;
                    return true;
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("SetPropertyFromJson - Failed to parse LinearColor from array"));
                }
            }
            // Additional check for string format, like when a string representation of an array is passed
            else if (JsonValue->Type == EJson::String)
            {
                UE_LOG(LogTemp, Log, TEXT("SetPropertyFromJson - Got string for FLinearColor: %s"), *JsonValue->AsString());
                FString ColorString = JsonValue->AsString();
                
                // Check if the string looks like an array: "[r, g, b, a]"
                if (ColorString.StartsWith(TEXT("[")) && ColorString.EndsWith(TEXT("]")))
                {
                    // Remove brackets
                    ColorString = ColorString.Mid(1, ColorString.Len() - 2);
                    
                    // Split by commas
                    TArray<FString> ColorComponents;
                    ColorString.ParseIntoArray(ColorComponents, TEXT(","), true);
                    
                    UE_LOG(LogTemp, Log, TEXT("SetPropertyFromJson - Parsed %d color components from string"), ColorComponents.Num());
                    
                    if (ColorComponents.Num() >= 3)
                    {
                        float R = FCString::Atof(*ColorComponents[0].TrimStart());
                        float G = FCString::Atof(*ColorComponents[1].TrimStart());
                        float B = FCString::Atof(*ColorComponents[2].TrimStart());
                        float A = ColorComponents.Num() >= 4 ? FCString::Atof(*ColorComponents[3].TrimStart()) : 1.0f;
                        
                        FLinearColor ColorValue(R, G, B, A);
                        UE_LOG(LogTemp, Log, TEXT("SetPropertyFromJson - Setting FLinearColor from string to (%f, %f, %f, %f)"), 
                            ColorValue.R, ColorValue.G, ColorValue.B, ColorValue.A);
                        *static_cast<FLinearColor*>(ContainerPtr) = ColorValue;
                        return true;
                    }
                    else
                    {
                        UE_LOG(LogTemp, Warning, TEXT("SetPropertyFromJson - Not enough color components in string: %s"), *ColorString);
                    }
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("SetPropertyFromJson - Color string is not in expected format: %s"), *ColorString);
                }
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("SetPropertyFromJson - Expected array or string for LinearColor but got different type"));
            }
        }
        // ... existing code ...
    }
    // ... existing code ...

    // Log failure if no suitable type handler was found
    UE_LOG(LogTemp, Warning, TEXT("SetPropertyFromJson: Unsupported property type '%s' or invalid JSON value type."), *Property->GetClass()->GetName());
    return false;
}

// Example implementation for ParseVector (adjust as needed)
bool FUnrealMCPCommonUtils::ParseVector(const TArray<TSharedPtr<FJsonValue>>& JsonArray, FVector& OutVector)
{
    if (JsonArray.Num() == 3) 
    {
        double X, Y, Z;
        if (JsonArray[0]->TryGetNumber(X) && JsonArray[1]->TryGetNumber(Y) && JsonArray[2]->TryGetNumber(Z))
        {
            OutVector.X = X;
            OutVector.Y = Y;
            OutVector.Z = Z;
            return true;
        }
    }
    return false;
}

// Example implementation for ParseLinearColor (adjust as needed)
bool FUnrealMCPCommonUtils::ParseLinearColor(const TArray<TSharedPtr<FJsonValue>>& JsonArray, FLinearColor& OutColor)
{
    UE_LOG(LogTemp, Log, TEXT("ParseLinearColor - Array has %d elements"), JsonArray.Num());
    
    if (JsonArray.Num() < 3)
    {
        UE_LOG(LogTemp, Warning, TEXT("ParseLinearColor - Array has insufficient elements: %d (need at least 3)"), JsonArray.Num());
        return false;
    }

    // Check each element to ensure they are numbers
    for (int32 i = 0; i < JsonArray.Num() && i < 4; ++i)
    {
        if (JsonArray[i]->Type != EJson::Number)
        {
            UE_LOG(LogTemp, Warning, TEXT("ParseLinearColor - Element %d is not a number (type: %d)"), i, (int)JsonArray[i]->Type);
            return false;
        }
    }

    // Extract RGB values
    float R = JsonArray[0]->AsNumber();
    float G = JsonArray[1]->AsNumber();
    float B = JsonArray[2]->AsNumber();
    
    // Extract Alpha if available, otherwise default to 1.0
    float A = 1.0f;
    if (JsonArray.Num() >= 4)
    {
        A = JsonArray[3]->AsNumber();
    }
    
    UE_LOG(LogTemp, Log, TEXT("ParseLinearColor - Parsed color: R=%f, G=%f, B=%f, A=%f"), R, G, B, A);
    
    // Set the output color
    OutColor = FLinearColor(R, G, B, A);
    return true;
}

// Placeholder for ParseRotator if needed
bool FUnrealMCPCommonUtils::ParseRotator(const TArray<TSharedPtr<FJsonValue>>& JsonArray, FRotator& OutRotator)
{
    if (JsonArray.Num() == 3) 
    {
        double P, Y, R;
        if (JsonArray[0]->TryGetNumber(P) && JsonArray[1]->TryGetNumber(Y) && JsonArray[2]->TryGetNumber(R))
        {
            OutRotator.Pitch = P;
            OutRotator.Yaw = Y;
            OutRotator.Roll = R;
            return true;
        }
    }
    return false;
}

// FindActorByName implementation might already exist, ensure it's suitable or adapt
AActor* FUnrealMCPCommonUtils::FindActorByName(const FString& ActorName)
{
    UWorld* World = GEditor->GetEditorWorldContext().World();
    if (!World) return nullptr;

    for (TActorIterator<AActor> It(World); It; ++It)
    {
        AActor* Actor = *It;
        if (Actor && Actor->GetName() == ActorName)
        {
            return Actor;
        }
    }
    return nullptr;
}

bool FUnrealMCPCommonUtils::CallFunctionByName(UObject* Target, const FString& FunctionName, const TArray<FString>& StringParams, FString& OutError)
{
    if (!Target) {
        OutError = TEXT("Target is null");
        return false;
    }
    UFunction* Function = Target->FindFunction(FName(*FunctionName));
    if (!Function) {
        OutError = FString::Printf(TEXT("Function not found: %s"), *FunctionName);
        return false;
    }
    uint8* Params = (uint8*)FMemory_Alloca(Function->ParmsSize);
    FMemory::Memzero(Params, Function->ParmsSize);

    int32 ParamIndex = 0;
    for (TFieldIterator<FProperty> It(Function); It && (It->PropertyFlags & CPF_Parm); ++It)
    {
        if (It->IsA<FStrProperty>() && ParamIndex < StringParams.Num())
        {
            FStrProperty* StrProp = CastField<FStrProperty>(*It);
            void* ValuePtr = It->ContainerPtrToValuePtr<void>(Params);
            StrProp->SetPropertyValue(ValuePtr, StringParams[ParamIndex]);
            ParamIndex++;
        }
        // Extend for other types as needed
    }
    Target->ProcessEvent(Function, Params);
    return true;
}