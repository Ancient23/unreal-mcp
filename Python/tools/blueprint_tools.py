"""
Blueprint Tools for Unreal MCP.

This module provides tools for creating and manipulating Blueprint assets in Unreal Engine.
"""

import logging
from typing import Dict, List, Any, Union
from mcp.server.fastmcp import FastMCP, Context
from utils.blueprints.blueprint_operations import (
    create_blueprint as create_blueprint_impl,
    add_component_to_blueprint as add_component_to_blueprint_impl,
    set_static_mesh_properties as set_static_mesh_properties_impl,
    set_component_property as set_component_property_impl,
    set_physics_properties as set_physics_properties_impl,
    compile_blueprint as compile_blueprint_impl,
    set_blueprint_property as set_blueprint_property_impl,
    set_pawn_properties as set_pawn_properties_impl,
    add_blueprint_custom_event_node as add_blueprint_custom_event_node_impl
)

# Get logger
logger = logging.getLogger("UnrealMCP")

def register_blueprint_tools(mcp: FastMCP):
    """Register Blueprint tools with the MCP server."""
    
    @mcp.tool()
    def create_blueprint(
        ctx: Context,
        name: str,
        parent_class: str,
        folder_path: str = ""
    ) -> Dict[str, Any]:
        """
        Create a new Blueprint class.
        
        Args:
            name: Name of the new Blueprint class (can include path with "/")
            parent_class: Parent class for the Blueprint (e.g., "Actor", "Pawn")
            folder_path: Optional folder path where the blueprint should be created
                         If name contains a path (e.g. "System/Blueprints/my_bp"), 
                         folder_path is ignored unless explicitly specified
                         
        Returns:
            Dictionary containing information about the created Blueprint including path and success status
        
        Examples:
            # Create blueprint directly in Content folder
            create_blueprint(name="MyBlueprint", parent_class="Actor")
            
            # Create blueprint with path in name (creates in Content/System/Blueprints)
            create_blueprint(name="System/Blueprints/my_bp", parent_class="Actor")
            
            # Create blueprint in Content/Success folder
            create_blueprint(name="MyBlueprint", parent_class="Actor", folder_path="Success")
        """
        return create_blueprint_impl(ctx, name, parent_class, folder_path)
    
    @mcp.tool()
    def add_component_to_blueprint(
        ctx: Context,
        blueprint_name: str,
        component_type: str,
        component_name: str,
        location: List[float] = None,
        rotation: List[float] = None,
        scale: List[float] = None,
        component_properties: Dict[str, Any] = None
    ) -> Dict[str, Any]:
        """
        Add a component to a Blueprint.
        
        Args:
            blueprint_name: Name of the target Blueprint
            component_type: Type of component to add (use component class name without U prefix)
            component_name: Name for the new component
            location: [X, Y, Z] coordinates for component's position
            rotation: [Pitch, Yaw, Roll] values for component's rotation
            scale: [X, Y, Z] values for component's scale
            component_properties: Additional properties to set on the component
        
        Returns:
            Information about the added component
        """
        return add_component_to_blueprint_impl(
            ctx, 
            blueprint_name, 
            component_type, 
            component_name, 
            location, 
            rotation, 
            scale, 
            component_properties
        )
    
    @mcp.tool()
    def set_static_mesh_properties(
        ctx: Context,
        blueprint_name: str,
        component_name: str,
        static_mesh: str = "/Engine/BasicShapes/Cube.Cube"
    ) -> Dict[str, Any]:
        """
        Set static mesh properties on a StaticMeshComponent.
        
        Args:
            blueprint_name: Name of the target Blueprint
            component_name: Name of the StaticMeshComponent
            static_mesh: Path to the static mesh asset (e.g., "/Engine/BasicShapes/Cube.Cube")
            
        Returns:
            Response indicating success or failure
        """
        return set_static_mesh_properties_impl(ctx, blueprint_name, component_name, static_mesh)
    
    @mcp.tool()
    def set_component_property(
        ctx: Context,
        blueprint_name: str,
        component_name: str,
        **kwargs
    ) -> Dict[str, Any]:
        """
        Set one or more properties on a component in a Blueprint.
        Args:
            blueprint_name: Name of the target Blueprint
            component_name: Name of the component
            kwargs: Properties to set (as keyword arguments or a dict). Each key is a property name, value is the value to set.
        Returns:
            Response indicating success or failure for each property.
        Example:
            set_component_property(ctx, "MyActor", "Mesh", StaticMesh="/Game/StarterContent/Shapes/Shape_Cube.Shape_Cube", Mobility="Movable")
        """
        return set_component_property_impl(ctx, blueprint_name, component_name, **kwargs)
    
    @mcp.tool()
    def set_physics_properties(
        ctx: Context,
        blueprint_name: str,
        component_name: str,
        simulate_physics: bool = True,
        gravity_enabled: bool = True,
        mass: float = 1.0,
        linear_damping: float = 0.01,
        angular_damping: float = 0.0
    ) -> Dict[str, Any]:
        """
        Set physics properties on a component.
        
        Args:
            blueprint_name: Name of the target Blueprint
            component_name: Name of the component
            simulate_physics: Whether to simulate physics on the component
            gravity_enabled: Whether gravity affects the component
            mass: Mass of the component in kg
            linear_damping: Linear damping factor
            angular_damping: Angular damping factor
            
        Returns:
            Response indicating success or failure
        """
        return set_physics_properties_impl(
            ctx, 
            blueprint_name, 
            component_name, 
            simulate_physics,
            gravity_enabled,
            mass,
            linear_damping,
            angular_damping
        )
    
    @mcp.tool()
    def compile_blueprint(
        ctx: Context,
        blueprint_name: str
    ) -> Dict[str, Any]:
        """
        Compile a Blueprint.
        
        Args:
            blueprint_name: Name of the target Blueprint
            
        Returns:
            Response indicating success or failure
        """
        return compile_blueprint_impl(ctx, blueprint_name)

    @mcp.tool()
    def set_blueprint_property(
        ctx: Context,
        blueprint_name: str,
        property_name: str,
        property_value: Any
    ) -> Dict[str, Any]:
        """
        Set a property on a Blueprint class default object.
        
        Args:
            blueprint_name: Name of the target Blueprint
            property_name: Name of the property to set
            property_value: Value to set the property to
            
        Returns:
            Response indicating success or failure
        """
        return set_blueprint_property_impl(ctx, blueprint_name, property_name, property_value)

    @mcp.tool()
    def set_pawn_properties(
        ctx: Context,
        blueprint_name: str,
        auto_possess_player: str = "",
        use_controller_rotation_yaw: bool = None,
        use_controller_rotation_pitch: bool = None,
        use_controller_rotation_roll: bool = None,
        can_be_damaged: bool = None
    ) -> Dict[str, Any]:
        """
        Set common Pawn properties on a Blueprint.
        This is a utility function that sets multiple pawn-related properties at once.
        
        Args:
            blueprint_name: Name of the target Blueprint (must be a Pawn or Character)
            auto_possess_player: Auto possess player setting (None, "Disabled", "Player0", "Player1", etc.)
            use_controller_rotation_yaw: Whether the pawn should use the controller's yaw rotation
            use_controller_rotation_pitch: Whether the pawn should use the controller's pitch rotation
            use_controller_rotation_roll: Whether the pawn should use the controller's roll rotation
            can_be_damaged: Whether the pawn can be damaged
            
        Returns:
            Response indicating success or failure with detailed results for each property
        """
        return set_pawn_properties_impl(
            ctx, 
            blueprint_name, 
            auto_possess_player,
            use_controller_rotation_yaw,
            use_controller_rotation_pitch,
            use_controller_rotation_roll,
            can_be_damaged
        )
    
    @mcp.tool()
    def add_blueprint_custom_event_node(
        ctx: Context,
        blueprint_name: str,
        event_name: str,
        node_position: List[float] = None
    ) -> Dict[str, Any]:
        """
        Adds a custom event node to the event graph of the specified Blueprint.
        Args:
            blueprint_name: Name of the target Blueprint
            event_name: Name of the custom event to create
            node_position: Optional [X, Y] position for the node
        Returns:
            Dict with node_id and event_name
        """
        return add_blueprint_custom_event_node_impl(ctx, blueprint_name, event_name, node_position)
    
    @mcp.tool()
    def call_blueprint_function(
        ctx: Context,
        target_name: str,
        function_name: str,
        string_params: list = None
    ) -> dict:
        """
        Call a BlueprintCallable function by name on the specified target.
        Only supports FString parameters for now.
        """
        payload = {
            "target_name": target_name,
            "function_name": function_name,
            "string_params": string_params or []
        }
        return ctx.mcp.command("call_function_by_name", payload)
    
    logger.info("Blueprint tools registered successfully")