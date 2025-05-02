#include "Commands/UnrealMCPBlueprintCommands.h"
#include "Commands/UnrealMCPCommonUtils.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Factories/BlueprintFactory.h"
#include "EdGraphSchema_K2.h"
#include "K2Node_Event.h"
#include "K2Node_VariableGet.h"
#include "K2Node_VariableSet.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Engine/SimpleConstructionScript.h"
#include "Engine/SCS_Node.h"
#include "UObject/Field.h"
#include "UObject/FieldPath.h"
#include "EditorAssetLibrary.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/GameModeBase.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Kismet/GameplayStatics.h"

// Using LogTemp instead of a custom log category for UE5.5 compatibility
// DEFINE_LOG_CATEGORY_STATIC(LogUnrealMCP, Log, All);

FUnrealMCPBlueprintCommands::FUnrealMCPBlueprintCommands()
{
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintCommands::HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params)
{
    if (CommandType == TEXT("create_blueprint"))
    {
        return HandleCreateBlueprint(Params);
    }
    else if (CommandType == TEXT("add_component_to_blueprint"))
    {
        return HandleAddComponentToBlueprint(Params);
    }
    else if (CommandType == TEXT("set_component_property"))
    {
        return HandleSetComponentProperty(Params);
    }
    else if (CommandType == TEXT("set_physics_properties"))
    {
        return HandleSetPhysicsProperties(Params);
    }
    else if (CommandType == TEXT("compile_blueprint"))
    {
        return HandleCompileBlueprint(Params);
    }
    else if (CommandType == TEXT("spawn_blueprint_actor"))
    {
        return HandleSpawnBlueprintActor(Params);
    }
    else if (CommandType == TEXT("set_blueprint_property"))
    {
        return HandleSetBlueprintProperty(Params);
    }
    else if (CommandType == TEXT("set_static_mesh_properties"))
    {
        return HandleSetStaticMeshProperties(Params);
    }
    else if (CommandType == TEXT("set_pawn_properties"))
    {
        return HandleSetPawnProperties(Params);
    }
    else if (CommandType == TEXT("call_function_by_name"))
    {
        return HandleCallFunctionByName(Params);
    }
    
    return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Unknown blueprint command: %s"), *CommandType));
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintCommands::HandleCreateBlueprint(const TSharedPtr<FJsonObject>& Params)
{
    // Get required parameters
    FString BlueprintFullPath;
    if (!Params->TryGetStringField(TEXT("name"), BlueprintFullPath))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'name' parameter"));
    }

    // Extract folder path and blueprint name if the name contains path separators
    FString BlueprintName;
    FString ProvidedFolderPath;
    
    if (BlueprintFullPath.Contains(TEXT("/")))
    {
        // Split path and name - find the last slash
        int32 LastSlashIndex = INDEX_NONE;
        if (BlueprintFullPath.FindLastChar('/', LastSlashIndex))
        {
            ProvidedFolderPath = BlueprintFullPath.Left(LastSlashIndex);
            BlueprintName = BlueprintFullPath.RightChop(LastSlashIndex + 1);
            UE_LOG(LogTemp, Log, TEXT("Extracted path from name: Path='%s', Name='%s'"), *ProvidedFolderPath, *BlueprintName);
        }
        else
        {
            BlueprintName = BlueprintFullPath;
        }
    }
    else
    {
        // No path separator in name
        BlueprintName = BlueprintFullPath;
    }

    // Get optional folder path parameter (default to empty path - Content folder)
    FString FolderPath;
    Params->TryGetStringField(TEXT("folder_path"), FolderPath);
    
    // If we already extracted a path from the name, use that unless folder_path was explicitly specified
    if (!ProvidedFolderPath.IsEmpty() && FolderPath.IsEmpty())
    {
        FolderPath = ProvidedFolderPath;
        UE_LOG(LogTemp, Log, TEXT("Using path extracted from name: '%s'"), *FolderPath);
    }

    // Normalize folder path - ensure it's format is "SomeFolder/SubFolder" without leading or trailing slashes
    if (FolderPath.StartsWith(TEXT("/")))
    {
        FolderPath = FolderPath.RightChop(1);
    }
    if (FolderPath.StartsWith(TEXT("Content/")))
    {
        FolderPath = FolderPath.RightChop(8); // Remove "Content/" prefix
    }
    if (FolderPath.StartsWith(TEXT("Game/")))
    {
        FolderPath = FolderPath.RightChop(5); // Remove "Game/" prefix
    }
    if (FolderPath.EndsWith(TEXT("/")))
    {
        FolderPath = FolderPath.LeftChop(1);
    }
    
    // Build the full package path - always start with /Game/ (Content folder)
    FString PackagePath = TEXT("/Game/");
    if (!FolderPath.IsEmpty())
    {
        PackagePath += FolderPath + TEXT("/");
    }
    
    // Check if the folder exists, and create it if it doesn't (recursively for nested paths)
    if (!FolderPath.IsEmpty() && !UEditorAssetLibrary::DoesDirectoryExist(PackagePath))
    {
        // For nested paths, we need to create each folder level recursively
        TArray<FString> FolderLevels;
        FolderPath.ParseIntoArray(FolderLevels, TEXT("/"));
        
        FString CurrentPath = TEXT("/Game/");
        for (const FString& FolderLevel : FolderLevels)
        {
            CurrentPath += FolderLevel + TEXT("/");
            if (!UEditorAssetLibrary::DoesDirectoryExist(CurrentPath))
            {
                UE_LOG(LogTemp, Log, TEXT("Creating directory: %s"), *CurrentPath);
                if (!UEditorAssetLibrary::MakeDirectory(CurrentPath))
                {
                    return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Failed to create directory: %s"), *CurrentPath));
                }
                UE_LOG(LogTemp, Log, TEXT("Created directory: %s"), *CurrentPath);
            }
        }
    }
    
    FString AssetName = BlueprintName;
    FString FullAssetPath = PackagePath + AssetName;
    
    // Check if blueprint already exists
    if (UEditorAssetLibrary::DoesAssetExist(FullAssetPath))
    {
        // Return success but indicate it already exists
        TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
        ResultObj->SetStringField(TEXT("name"), BlueprintFullPath); // Return the original full path name that was passed in
        ResultObj->SetStringField(TEXT("path"), FullAssetPath);
        ResultObj->SetBoolField(TEXT("already_exists"), true);
        return ResultObj;
    }

    // Create the blueprint factory
    UBlueprintFactory* Factory = NewObject<UBlueprintFactory>();
    
    // Handle parent class
    FString ParentClass;
    Params->TryGetStringField(TEXT("parent_class"), ParentClass);
    
    // Default to Actor if no parent class specified
    UClass* SelectedParentClass = AActor::StaticClass();
    
    // Try to find the specified parent class
    if (!ParentClass.IsEmpty())
    {
        FString ClassName = ParentClass;
        // Handle both Actor and ActorComponent classes
        if (!ClassName.StartsWith(TEXT("A")) && !ClassName.StartsWith(TEXT("U")))
        {
            // If it ends with "Component", add U prefix, otherwise add A prefix
            if (ClassName.EndsWith(TEXT("Component")))
            {
                ClassName = TEXT("U") + ClassName;
            }
            else
            {
                ClassName = TEXT("A") + ClassName;
            }
        }
        
        // First try direct StaticClass lookup for common classes
        UClass* FoundClass = nullptr;
        if (ClassName == TEXT("APawn"))
        {
            FoundClass = APawn::StaticClass();
        }
        else if (ClassName == TEXT("AActor"))
        {
            FoundClass = AActor::StaticClass();
        }
        else if (ClassName == TEXT("UActorComponent"))
        {
            FoundClass = UActorComponent::StaticClass();
        }
        else if (ClassName == TEXT("USceneComponent"))
        {
            FoundClass = USceneComponent::StaticClass();
        }
        else if (ClassName == TEXT("ACharacter"))
        {
            FoundClass = ACharacter::StaticClass();
        }
        else if (ClassName == TEXT("APlayerController"))
        {
            FoundClass = APlayerController::StaticClass();
        }
        else if (ClassName == TEXT("AGameModeBase"))
        {
            FoundClass = AGameModeBase::StaticClass();
        }
        else
        {
            // Try loading the class using LoadClass which is more reliable than FindObject
            // Try common module paths
            TArray<FString> ModulePaths = {
                TEXT("/Script/Engine"),
                TEXT("/Script/GameplayAbilities"),
                TEXT("/Script/AIModule"),
                TEXT("/Script/Game"),
                TEXT("/Script/CoreUObject")
            };

            for (const FString& ModulePath : ModulePaths)
            {
                const FString ClassPath = FString::Printf(TEXT("%s.%s"), *ModulePath, *ClassName);
                FoundClass = LoadClass<UObject>(nullptr, *ClassPath);
                if (FoundClass)
                {
                    break;
                }
            }
        }

        if (FoundClass)
        {
            SelectedParentClass = FoundClass;
            UE_LOG(LogTemp, Log, TEXT("Successfully set parent class to '%s'"), *ClassName);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Could not find specified parent class '%s' at paths: /Script/Engine.%s or /Script/Game.%s, defaulting to AActor"), 
                *ClassName, *ClassName, *ClassName);
        }
    }
    
    Factory->ParentClass = SelectedParentClass;

    // Add extra logging to debug creation
    UE_LOG(LogTemp, Log, TEXT("Creating blueprint: Name='%s', FullPath='%s', ParentClass='%s'"), 
        *AssetName, *FullAssetPath, *SelectedParentClass->GetName());

    // Create the package
    UPackage* Package = CreatePackage(*FullAssetPath);
    if (!Package)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create package for path: '%s'"), *FullAssetPath);
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Failed to create package for path: '%s'"), *FullAssetPath));
    }

    // Create the blueprint using FKismetEditorUtilities which handles all necessary setup
    UBlueprint* NewBlueprint = FKismetEditorUtilities::CreateBlueprint(
        SelectedParentClass,
        Package,
        *AssetName,
        BPTYPE_Normal,
        UBlueprint::StaticClass(),
        UBlueprintGeneratedClass::StaticClass(),
        NAME_None
    );

    if (NewBlueprint)
    {
        // Notify the asset registry
        FAssetRegistryModule::AssetCreated(NewBlueprint);

        // Mark the package dirty
        Package->MarkPackageDirty();
        
        // Save the asset to disk to ensure it persists
        if (UEditorAssetLibrary::SaveLoadedAsset(NewBlueprint))
        {
            UE_LOG(LogTemp, Log, TEXT("Successfully saved blueprint: %s"), *FullAssetPath);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to save blueprint: %s"), *FullAssetPath);
        }

        TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
        ResultObj->SetStringField(TEXT("name"), BlueprintFullPath); // Return the original full path name that was passed in
        ResultObj->SetStringField(TEXT("path"), FullAssetPath);
        ResultObj->SetBoolField(TEXT("already_exists"), false);
        return ResultObj;
    }

    return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create blueprint"));
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintCommands::HandleAddComponentToBlueprint(const TSharedPtr<FJsonObject>& Params)
{
    // Get required parameters
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    FString ComponentType;
    if (!Params->TryGetStringField(TEXT("component_type"), ComponentType))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'type' parameter"));
    }

    FString ComponentName;
    if (!Params->TryGetStringField(TEXT("component_name"), ComponentName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'name' parameter"));
    }

    // Find the blueprint
    UBlueprint* Blueprint = FUnrealMCPCommonUtils::FindBlueprint(BlueprintName);
    if (!Blueprint)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));
    }

    // Create the component - dynamically find the component class by name
    UClass* ComponentClass = nullptr;
    
    // Log what we're trying to find for debugging
    UE_LOG(LogTemp, Display, TEXT("Attempting to find component class: %s"), *ComponentType);

    // Check if it's a direct match for one of the known components from our log
    // This is a direct fix for the issue we're seeing
    if (ComponentType == TEXT("StaticMesh"))
    {
        FString DirectComponentName = TEXT("StaticMeshComponent");
        // Try to find the component class using various paths
        TArray<FString> ClassPaths;
        ClassPaths.Add(FUnrealMCPCommonUtils::BuildEnginePath(DirectComponentName));
        ClassPaths.Add(FUnrealMCPCommonUtils::BuildCorePath(DirectComponentName));
        ClassPaths.Add(FUnrealMCPCommonUtils::BuildGamePath(FString::Printf(TEXT("Blueprints/%s.%s_C"), *DirectComponentName, *DirectComponentName)));
        ClassPaths.Add(FUnrealMCPCommonUtils::BuildGamePath(FString::Printf(TEXT("%s.%s_C"), *DirectComponentName, *DirectComponentName)));

        for (const FString& ClassPath : ClassPaths)
        {
            ComponentClass = LoadObject<UClass>(nullptr, *ClassPath);
            if (ComponentClass)
            {
                break;
            }
        }

        // Try with different prefixes/suffixes if not found
        if (!ComponentClass)
        {
            TArray<FString> VariationPaths;
            // With U prefix
            FString WithUPrefix = FString::Printf(TEXT("U%s"), *ComponentType);
            VariationPaths.Add(FUnrealMCPCommonUtils::BuildEnginePath(WithUPrefix));
            VariationPaths.Add(FUnrealMCPCommonUtils::BuildCorePath(WithUPrefix));
            
            // With Component suffix
            FString WithSuffix = FString::Printf(TEXT("%sComponent"), *ComponentType);
            VariationPaths.Add(FUnrealMCPCommonUtils::BuildEnginePath(WithSuffix));
            VariationPaths.Add(FUnrealMCPCommonUtils::BuildCorePath(WithSuffix));
            
            // With U prefix and Component suffix
            FString WithBoth = FString::Printf(TEXT("U%sComponent"), *ComponentType);
            VariationPaths.Add(FUnrealMCPCommonUtils::BuildEnginePath(WithBoth));
            VariationPaths.Add(FUnrealMCPCommonUtils::BuildCorePath(WithBoth));

            for (const FString& Path : VariationPaths)
            {
                ComponentClass = LoadObject<UClass>(nullptr, *Path);
                if (ComponentClass)
                {
                    break;
                }
            }
        }
    }
    
    // If still not found, try matching against common component types
    if (!ComponentClass)
    {
        // Common component type mappings - use exact matches from the log
        TMap<FString, FString> CommonComponentTypes;
        CommonComponentTypes.Add(TEXT("StaticMesh"), TEXT("StaticMeshComponent"));
        CommonComponentTypes.Add(TEXT("PointLight"), TEXT("PointLightComponent"));
        CommonComponentTypes.Add(TEXT("SpotLight"), TEXT("SpotLightComponent"));
        CommonComponentTypes.Add(TEXT("DirectionalLight"), TEXT("DirectionalLightComponent"));
        CommonComponentTypes.Add(TEXT("Box"), TEXT("BoxComponent"));
        CommonComponentTypes.Add(TEXT("Sphere"), TEXT("SphereComponent"));
        CommonComponentTypes.Add(TEXT("Capsule"), TEXT("CapsuleComponent"));
        CommonComponentTypes.Add(TEXT("Camera"), TEXT("CameraComponent"));
        CommonComponentTypes.Add(TEXT("Audio"), TEXT("AudioComponent"));
        CommonComponentTypes.Add(TEXT("Scene"), TEXT("SceneComponent"));
        CommonComponentTypes.Add(TEXT("Billboard"), TEXT("BillboardComponent"));
        
        // Check if we have a mapping for this component type
        FString* MappedType = CommonComponentTypes.Find(ComponentType);
        if (MappedType)
        {
            // Try loading from Engine module first
            FString EnginePath = FUnrealMCPCommonUtils::BuildEnginePath(**MappedType);
            ComponentClass = LoadObject<UClass>(nullptr, *EnginePath);
            
            if (!ComponentClass)
            {
                // Try other common modules if not found in Engine
                TArray<FString> ModulePaths = {
                    FUnrealMCPCommonUtils::BuildCorePath(TEXT("UMG")),
                    FUnrealMCPCommonUtils::BuildCorePath(TEXT("NavigationSystem")),
                    FUnrealMCPCommonUtils::BuildCorePath(TEXT("AIModule"))
                };
                
                for (const FString& ModulePath : ModulePaths)
                {
                    if (ComponentClass) break;
                    FString FullPath = FUnrealMCPCommonUtils::BuildCorePath(FString::Printf(TEXT("%s.%s"), *ModulePath, **MappedType));
                    ComponentClass = LoadObject<UClass>(nullptr, *FullPath);
                }
            }
            
            UE_LOG(LogTemp, Display, TEXT("Mapped to common type: %s -> %s, Found: %s"), 
                  *ComponentType, **MappedType, ComponentClass ? TEXT("Yes") : TEXT("No"));
        }
    }
    
    // If still not found, try direct loading by path
    if (!ComponentClass)
    {
        // Try direct loading by path for engine components
        FString EnginePath = FUnrealMCPCommonUtils::BuildEnginePath(FString::Printf(TEXT("%sComponent"), *ComponentType));
        ComponentClass = LoadObject<UClass>(nullptr, *EnginePath);
        UE_LOG(LogTemp, Display, TEXT("LoadObject<%s>: %s"), *EnginePath, ComponentClass ? TEXT("Found") : TEXT("Not found"));
        
        if (!ComponentClass)
        {
            // For components that don't follow the standard naming pattern
            FString RawPath = FUnrealMCPCommonUtils::BuildEnginePath(FString::Printf(TEXT("%s"), *ComponentType));
            ComponentClass = LoadObject<UClass>(nullptr, *RawPath);
            UE_LOG(LogTemp, Display, TEXT("LoadObject<%s>: %s"), *RawPath, ComponentClass ? TEXT("Found") : TEXT("Not found"));
        }
    }
    
    // Verify that the class is a valid component type
    if (!ComponentClass || !ComponentClass->IsChildOf(UActorComponent::StaticClass()))
    {
        // List available component classes for debugging
        UE_LOG(LogTemp, Error, TEXT("Failed to find component class for: %s"), *ComponentType);
        UE_LOG(LogTemp, Error, TEXT("Listing first 10 available component classes:"));
        int32 Count = 0;
        for (TObjectIterator<UClass> It; It && Count < 10; ++It)
        {
            UClass* Class = *It;
            if (Class->IsChildOf(UActorComponent::StaticClass()))
            {
                UE_LOG(LogTemp, Error, TEXT("  - %s"), *Class->GetName());
                Count++;
            }
        }
        
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(
            TEXT("Unknown component type: %s. Please check logs for available component types."), *ComponentType));
    }

    // Add the component to the blueprint
    USCS_Node* NewNode = Blueprint->SimpleConstructionScript->CreateNode(ComponentClass, *ComponentName);
    if (NewNode)
    {
        // Set transform if provided
        USceneComponent* SceneComponent = Cast<USceneComponent>(NewNode->ComponentTemplate);
        if (SceneComponent)
        {
            if (Params->HasField(TEXT("location")))
            {
                SceneComponent->SetRelativeLocation(FUnrealMCPCommonUtils::GetVectorFromJson(Params, TEXT("location")));
            }
            if (Params->HasField(TEXT("rotation")))
            {
                SceneComponent->SetRelativeRotation(FUnrealMCPCommonUtils::GetRotatorFromJson(Params, TEXT("rotation")));
            }
            if (Params->HasField(TEXT("scale")))
            {
                SceneComponent->SetRelativeScale3D(FUnrealMCPCommonUtils::GetVectorFromJson(Params, TEXT("scale")));
            }
        }

        // Add to root if no parent specified
        Blueprint->SimpleConstructionScript->AddNode(NewNode);

        // Compile the blueprint
        FKismetEditorUtilities::CompileBlueprint(Blueprint);

        TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
        ResultObj->SetStringField(TEXT("component_name"), ComponentName);
        ResultObj->SetStringField(TEXT("component_type"), ComponentType);
        ResultObj->SetStringField(TEXT("actual_class"), ComponentClass->GetName());
        return ResultObj;
    }

    return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to add component to blueprint"));
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintCommands::HandleSetComponentProperty(const TSharedPtr<FJsonObject>& Params)
{
    // Get required parameters
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    FString ComponentName;
    if (!Params->TryGetStringField(TEXT("component_name"), ComponentName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'component_name' parameter"));
    }

    // Get kwargs (properties to set)
    TSharedPtr<FJsonObject> KwargsObject;
    const TSharedPtr<FJsonObject>* KwargsObjectPtr = nullptr;
    // Try to get as object first
    if (Params->TryGetObjectField(TEXT("kwargs"), KwargsObjectPtr) && KwargsObjectPtr && KwargsObjectPtr->IsValid()) {
        KwargsObject = *KwargsObjectPtr;
    } else {
        // Try to get as string and parse
        FString KwargsString;
        if (Params->TryGetStringField(TEXT("kwargs"), KwargsString)) {
            TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(KwargsString);
            TSharedPtr<FJsonObject> ParsedObject;
            if (FJsonSerializer::Deserialize(Reader, ParsedObject) && ParsedObject.IsValid()) {
                KwargsObject = ParsedObject;
            }
        }
    }
    if (!KwargsObject.IsValid()) {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing or invalid 'kwargs' parameter (must be a dictionary of properties or a JSON string)"));
    }

    // Find the blueprint
    UBlueprint* Blueprint = FUnrealMCPCommonUtils::FindBlueprint(BlueprintName);
    if (!Blueprint)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));
    }

    // Find the component
    USCS_Node* ComponentNode = nullptr;
    if (!Blueprint->SimpleConstructionScript)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Invalid blueprint construction script"));
    }
    for (USCS_Node* Node : Blueprint->SimpleConstructionScript->GetAllNodes())
    {
        if (Node && Node->GetVariableName().ToString() == ComponentName)
        {
            ComponentNode = Node;
            break;
        }
    }
    if (!ComponentNode)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Component not found: %s"), *ComponentName));
    }
    UObject* ComponentTemplate = ComponentNode->ComponentTemplate;
    if (!ComponentTemplate)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Invalid component template"));
    }

    // Loop over all properties in kwargs
    TArray<FString> PropertyNames;
    KwargsObject->Values.GetKeys(PropertyNames);
    TArray<FString> SuccessProps;
    TMap<FString, FString> FailedProps;
    for (const FString& PropertyName : PropertyNames)
    {
        TSharedPtr<FJsonValue> PropertyValue = KwargsObject->TryGetField(PropertyName);

        // --- Custom handling for collision properties ---
        bool bHandled = false;
        UPrimitiveComponent* PrimComponent = Cast<UPrimitiveComponent>(ComponentTemplate);
        if (PrimComponent)
        {
            if (PropertyName == TEXT("CollisionEnabled"))
            {
                FString ValueString;
                if (PropertyValue->TryGetString(ValueString))
                {
                    // Map string to ECollisionEnabled::Type
                    ECollisionEnabled::Type CollisionType = ECollisionEnabled::NoCollision;
                    if (ValueString == "NoCollision") CollisionType = ECollisionEnabled::NoCollision;
                    else if (ValueString == "QueryOnly") CollisionType = ECollisionEnabled::QueryOnly;
                    else if (ValueString == "PhysicsOnly") CollisionType = ECollisionEnabled::PhysicsOnly;
                    else if (ValueString == "QueryAndPhysics") CollisionType = ECollisionEnabled::QueryAndPhysics;
                    PrimComponent->SetCollisionEnabled(CollisionType);
                    SuccessProps.Add(PropertyName);
                    bHandled = true;
                }
            }
            else if (PropertyName == TEXT("CollisionProfileName"))
            {
                FString ValueString;
                if (PropertyValue->TryGetString(ValueString))
                {
                    PrimComponent->SetCollisionProfileName(*ValueString);
                    SuccessProps.Add(PropertyName);
                    bHandled = true;
                }
            }
        }
        if (bHandled)
            continue;
        // --- End custom handling ---

        FProperty* Property = FindFProperty<FProperty>(ComponentTemplate->GetClass(), *PropertyName);
        if (!Property)
        {
            FailedProps.Add(PropertyName, FString::Printf(TEXT("Property '%s' not found on component '%s' (Class: %s)"), *PropertyName, *ComponentName, *ComponentTemplate->GetClass()->GetName()));
            continue;
        }
        void* PropertyData = Property->ContainerPtrToValuePtr<void>(ComponentTemplate);
        FString ErrorMessage;
        if (!FUnrealMCPCommonUtils::SetPropertyFromJson(Property, PropertyData, PropertyValue))
        {
            FailedProps.Add(PropertyName, FString::Printf(TEXT("Failed to set property '%s'. Check value type/format."), *PropertyName));
            continue;
        }
        SuccessProps.Add(PropertyName);
    }

    // If any property was set, mark blueprint as modified and compile
    if (SuccessProps.Num() > 0)
    {
        FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
        FKismetEditorUtilities::CompileBlueprint(Blueprint);
    }

    // Build response
    TSharedPtr<FJsonObject> Response = FUnrealMCPCommonUtils::CreateSuccessResponse();
    Response->SetStringField(TEXT("blueprint_name"), BlueprintName);
    Response->SetStringField(TEXT("component_name"), ComponentName);
    TArray<TSharedPtr<FJsonValue>> SuccessArray;
    for (const FString& Prop : SuccessProps)
    {
        SuccessArray.Add(MakeShared<FJsonValueString>(Prop));
    }
    Response->SetArrayField(TEXT("success_properties"), SuccessArray);
    TArray<TSharedPtr<FJsonValue>> FailedArray;
    for (const auto& Pair : FailedProps)
    {
        TSharedPtr<FJsonObject> FailObj = MakeShared<FJsonObject>();
        FailObj->SetStringField(TEXT("property"), Pair.Key);
        FailObj->SetStringField(TEXT("error"), Pair.Value);
        FailedArray.Add(MakeShared<FJsonValueObject>(FailObj));
    }
    Response->SetArrayField(TEXT("failed_properties"), FailedArray);
    return Response;
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintCommands::HandleSetPhysicsProperties(const TSharedPtr<FJsonObject>& Params)
{
    // Get required parameters
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    FString ComponentName;
    if (!Params->TryGetStringField(TEXT("component_name"), ComponentName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'component_name' parameter"));
    }

    // Find the blueprint
    UBlueprint* Blueprint = FUnrealMCPCommonUtils::FindBlueprint(BlueprintName);
    if (!Blueprint)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));
    }

    // Find the component
    USCS_Node* ComponentNode = nullptr;
    for (USCS_Node* Node : Blueprint->SimpleConstructionScript->GetAllNodes())
    {
        if (Node && Node->GetVariableName().ToString() == ComponentName)
        {
            ComponentNode = Node;
            break;
        }
    }

    if (!ComponentNode)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Component not found: %s"), *ComponentName));
    }

    UPrimitiveComponent* PrimComponent = Cast<UPrimitiveComponent>(ComponentNode->ComponentTemplate);
    if (!PrimComponent)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Component is not a primitive component"));
    }

    // Set physics properties
    if (Params->HasField(TEXT("simulate_physics")))
    {
        PrimComponent->SetSimulatePhysics(Params->GetBoolField(TEXT("simulate_physics")));
    }

    if (Params->HasField(TEXT("mass")))
    {
        float Mass = Params->GetNumberField(TEXT("mass"));
        // In UE5.5, use proper overrideMass instead of just scaling
        PrimComponent->SetMassOverrideInKg(NAME_None, Mass);
        UE_LOG(LogTemp, Display, TEXT("Set mass for component %s to %f kg"), *ComponentName, Mass);
    }

    if (Params->HasField(TEXT("linear_damping")))
    {
        PrimComponent->SetLinearDamping(Params->GetNumberField(TEXT("linear_damping")));
    }

    if (Params->HasField(TEXT("angular_damping")))
    {
        PrimComponent->SetAngularDamping(Params->GetNumberField(TEXT("angular_damping")));
    }

    // Mark the blueprint as modified
    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetStringField(TEXT("component"), ComponentName);
    return ResultObj;
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintCommands::HandleCompileBlueprint(const TSharedPtr<FJsonObject>& Params)
{
    // Get required parameters
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    // Find the blueprint
    UBlueprint* Blueprint = FUnrealMCPCommonUtils::FindBlueprint(BlueprintName);
    if (!Blueprint)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));
    }

    // Compile the blueprint
    FKismetEditorUtilities::CompileBlueprint(Blueprint);

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetStringField(TEXT("name"), BlueprintName);
    ResultObj->SetBoolField(TEXT("compiled"), true);
    return ResultObj;
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintCommands::HandleSpawnBlueprintActor(const TSharedPtr<FJsonObject>& Params)
{
    
    // Get required parameters
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    FString ActorName;
    if (!Params->TryGetStringField(TEXT("actor_name"), ActorName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'actor_name' parameter"));
    }

    // Find the blueprint
    UBlueprint* Blueprint = FUnrealMCPCommonUtils::FindBlueprint(BlueprintName);
    if (!Blueprint)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));
    }
    
    
    // Try to compile the blueprint if needed
    if (!Blueprint->GeneratedClass)
    {
        FKismetEditorUtilities::CompileBlueprint(Blueprint);
        
        // Check again after compilation
        if (!Blueprint->GeneratedClass)
        {
            return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Blueprint compilation failed"));
        }
    }

    // Get transform parameters
    FVector Location(0.0f, 0.0f, 0.0f);
    FRotator Rotation(0.0f, 0.0f, 0.0f);

    if (Params->HasField(TEXT("location")))
    {
        Location = FUnrealMCPCommonUtils::GetVectorFromJson(Params, TEXT("location"));
    }
    if (Params->HasField(TEXT("rotation")))
    {
        Rotation = FUnrealMCPCommonUtils::GetRotatorFromJson(Params, TEXT("rotation"));
    }

    // Spawn the actor
    UWorld* World = GEditor->GetEditorWorldContext().World();
    if (!World)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to get editor world"));
    }

    FTransform SpawnTransform;
    SpawnTransform.SetLocation(Location);
    SpawnTransform.SetRotation(FQuat(Rotation));
    
    // Extra validation of GeneratedClass
    if (!Blueprint->GeneratedClass)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Blueprint's GeneratedClass is null"));
    }
    
    // Check if the blueprint is properly compiled
    if (!Blueprint->GeneratedClass->IsValidLowLevel())
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Blueprint GeneratedClass is not valid"));
    }
    
    // Additional checks for blueprint class properties
    UBlueprintGeneratedClass* BPGenClass = Cast<UBlueprintGeneratedClass>(Blueprint->GeneratedClass);

    // Try spawning with SpawnActorDeferred first for better diagnostics
    AActor* NewActor = nullptr;
    try
    {
        
        // Capture actual class pointer before spawning
        UClass* ClassToSpawn = Blueprint->GeneratedClass;
        
        // First try with deferred spawning
        NewActor = World->SpawnActorDeferred<AActor>(ClassToSpawn, SpawnTransform);
        if (NewActor)
        {
            UGameplayStatics::FinishSpawningActor(NewActor, SpawnTransform);
        }
        else
        {
            
            // Fallback to regular spawn
            NewActor = World->SpawnActor<AActor>(ClassToSpawn, SpawnTransform);
        }
    }
    catch (const std::exception& e)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(
            FString::Printf(TEXT("Exception during spawn: %s"), UTF8_TO_TCHAR(e.what())));
    }
    catch (...)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Unknown exception during spawn"));
    }

    if (NewActor)
    {
        NewActor->SetActorLabel(*ActorName);
        return FUnrealMCPCommonUtils::ActorToJsonObject(NewActor, true);
    }

    return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to spawn blueprint actor"));
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintCommands::HandleSetBlueprintProperty(const TSharedPtr<FJsonObject>& Params)
{
    // Get required parameters
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    FString PropertyName;
    if (!Params->TryGetStringField(TEXT("property_name"), PropertyName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'property_name' parameter"));
    }

    // Find the blueprint
    UBlueprint* Blueprint = FUnrealMCPCommonUtils::FindBlueprint(BlueprintName);
    if (!Blueprint)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));
    }

    // Get the default object
    UObject* DefaultObject = Blueprint->GeneratedClass->GetDefaultObject();
    if (!DefaultObject)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to get default object"));
    }

    // Set the property value
    if (Params->HasField(TEXT("property_value")))
    {
        TSharedPtr<FJsonValue> JsonValue = Params->Values.FindRef(TEXT("property_value"));
        
        FString ErrorMessage;
        if (FUnrealMCPCommonUtils::SetObjectProperty(DefaultObject, PropertyName, JsonValue, ErrorMessage))
        {
            // Mark the blueprint as modified
            FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

            TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
            ResultObj->SetStringField(TEXT("property"), PropertyName);
            ResultObj->SetBoolField(TEXT("success"), true);
            return ResultObj;
        }
        else
        {
            return FUnrealMCPCommonUtils::CreateErrorResponse(ErrorMessage);
        }
    }

    return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'property_value' parameter"));
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintCommands::HandleSetStaticMeshProperties(const TSharedPtr<FJsonObject>& Params)
{
    // Get required parameters
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    FString ComponentName;
    if (!Params->TryGetStringField(TEXT("component_name"), ComponentName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'component_name' parameter"));
    }

    // Find the blueprint
    UBlueprint* Blueprint = FUnrealMCPCommonUtils::FindBlueprint(BlueprintName);
    if (!Blueprint)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));
    }

    // Find the component
    USCS_Node* ComponentNode = nullptr;
    for (USCS_Node* Node : Blueprint->SimpleConstructionScript->GetAllNodes())
    {
        if (Node && Node->GetVariableName().ToString() == ComponentName)
        {
            ComponentNode = Node;
            break;
        }
    }

    if (!ComponentNode)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Component not found: %s"), *ComponentName));
    }

    UStaticMeshComponent* MeshComponent = Cast<UStaticMeshComponent>(ComponentNode->ComponentTemplate);
    if (!MeshComponent)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Component is not a static mesh component"));
    }

    // Set static mesh properties
    if (Params->HasField(TEXT("static_mesh")))
    {
        FString MeshPath = Params->GetStringField(TEXT("static_mesh"));
        UStaticMesh* Mesh = Cast<UStaticMesh>(UEditorAssetLibrary::LoadAsset(MeshPath));
        if (Mesh)
        {
            MeshComponent->SetStaticMesh(Mesh);
        }
    }

    if (Params->HasField(TEXT("material")))
    {
        FString MaterialPath = Params->GetStringField(TEXT("material"));
        UMaterialInterface* Material = Cast<UMaterialInterface>(UEditorAssetLibrary::LoadAsset(MaterialPath));
        if (Material)
        {
            MeshComponent->SetMaterial(0, Material);
        }
    }

    // Mark the blueprint as modified
    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetStringField(TEXT("component"), ComponentName);
    return ResultObj;
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintCommands::HandleSetPawnProperties(const TSharedPtr<FJsonObject>& Params)
{
    // Get required parameters
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    // Find the blueprint
    UBlueprint* Blueprint = FUnrealMCPCommonUtils::FindBlueprint(BlueprintName);
    if (!Blueprint)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));
    }

    // Get the default object
    UObject* DefaultObject = Blueprint->GeneratedClass->GetDefaultObject();
    if (!DefaultObject)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to get default object"));
    }

    // Track if any properties were set successfully
    bool bAnyPropertiesSet = false;
    TSharedPtr<FJsonObject> ResultsObj = MakeShared<FJsonObject>();
    
    // Set auto possess player if specified
    if (Params->HasField(TEXT("auto_possess_player")))
    {
        TSharedPtr<FJsonValue> AutoPossessValue = Params->Values.FindRef(TEXT("auto_possess_player"));
        
        FString ErrorMessage;
        if (FUnrealMCPCommonUtils::SetObjectProperty(DefaultObject, TEXT("AutoPossessPlayer"), AutoPossessValue, ErrorMessage))
        {
            bAnyPropertiesSet = true;
            TSharedPtr<FJsonObject> PropResultObj = MakeShared<FJsonObject>();
            PropResultObj->SetBoolField(TEXT("success"), true);
            ResultsObj->SetObjectField(TEXT("AutoPossessPlayer"), PropResultObj);
        }
        else
        {
            TSharedPtr<FJsonObject> PropResultObj = MakeShared<FJsonObject>();
            PropResultObj->SetBoolField(TEXT("success"), false);
            PropResultObj->SetStringField(TEXT("error"), ErrorMessage);
            ResultsObj->SetObjectField(TEXT("AutoPossessPlayer"), PropResultObj);
        }
    }
    
    // Set controller rotation properties
    const TCHAR* RotationProps[] = {
        TEXT("bUseControllerRotationYaw"),
        TEXT("bUseControllerRotationPitch"),
        TEXT("bUseControllerRotationRoll")
    };
    
    const TCHAR* ParamNames[] = {
        TEXT("use_controller_rotation_yaw"),
        TEXT("use_controller_rotation_pitch"),
        TEXT("use_controller_rotation_roll")
    };
    
    for (int32 i = 0; i < 3; i++)
    {
        if (Params->HasField(ParamNames[i]))
        {
            TSharedPtr<FJsonValue> Value = Params->Values.FindRef(ParamNames[i]);
            
            FString ErrorMessage;
            if (FUnrealMCPCommonUtils::SetObjectProperty(DefaultObject, RotationProps[i], Value, ErrorMessage))
            {
                bAnyPropertiesSet = true;
                TSharedPtr<FJsonObject> PropResultObj = MakeShared<FJsonObject>();
                PropResultObj->SetBoolField(TEXT("success"), true);
                ResultsObj->SetObjectField(RotationProps[i], PropResultObj);
            }
            else
            {
                TSharedPtr<FJsonObject> PropResultObj = MakeShared<FJsonObject>();
                PropResultObj->SetBoolField(TEXT("success"), false);
                PropResultObj->SetStringField(TEXT("error"), ErrorMessage);
                ResultsObj->SetObjectField(RotationProps[i], PropResultObj);
            }
        }
    }
    
    // Set can be damaged property
    if (Params->HasField(TEXT("can_be_damaged")))
    {
        TSharedPtr<FJsonValue> Value = Params->Values.FindRef(TEXT("can_be_damaged"));
        
        FString ErrorMessage;
        if (FUnrealMCPCommonUtils::SetObjectProperty(DefaultObject, TEXT("bCanBeDamaged"), Value, ErrorMessage))
        {
            bAnyPropertiesSet = true;
            TSharedPtr<FJsonObject> PropResultObj = MakeShared<FJsonObject>();
            PropResultObj->SetBoolField(TEXT("success"), true);
            ResultsObj->SetObjectField(TEXT("bCanBeDamaged"), PropResultObj);
        }
        else
        {
            TSharedPtr<FJsonObject> PropResultObj = MakeShared<FJsonObject>();
            PropResultObj->SetBoolField(TEXT("success"), false);
            PropResultObj->SetStringField(TEXT("error"), ErrorMessage);
            ResultsObj->SetObjectField(TEXT("bCanBeDamaged"), PropResultObj);
        }
    }

    // Mark the blueprint as modified if any properties were set
    if (bAnyPropertiesSet)
    {
        FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
    }
    else if (ResultsObj->Values.Num() == 0)
    {
        // No properties were specified
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("No properties specified to set"));
    }

    TSharedPtr<FJsonObject> ResponseObj = MakeShared<FJsonObject>();
    ResponseObj->SetStringField(TEXT("blueprint"), BlueprintName);
    ResponseObj->SetBoolField(TEXT("success"), bAnyPropertiesSet);
    ResponseObj->SetObjectField(TEXT("results"), ResultsObj);
    return ResponseObj;
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintCommands::HandleCallFunctionByName(const TSharedPtr<FJsonObject>& Params)
{
    FString TargetName, FunctionName;
    TArray<FString> StringParams;
    if (!Params->TryGetStringField(TEXT("target_name"), TargetName) ||
        !Params->TryGetStringField(TEXT("function_name"), FunctionName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing required parameters"));
    }
    // Find the target object (for demo: try actor, then nullptr)
    UObject* Target = FUnrealMCPCommonUtils::FindActorByName(TargetName); // Extend as needed
    if (!Target)
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Target not found"));

    // Parse string_params array
    const TArray<TSharedPtr<FJsonValue>>* JsonParams;
    if (Params->TryGetArrayField(TEXT("string_params"), JsonParams))
    {
        for (const auto& Val : *JsonParams)
            StringParams.Add(Val->AsString());
    }

    FString Error;
    if (!FUnrealMCPCommonUtils::CallFunctionByName(Target, FunctionName, StringParams, Error))
        return FUnrealMCPCommonUtils::CreateErrorResponse(Error);

    return FUnrealMCPCommonUtils::CreateSuccessResponse();
}