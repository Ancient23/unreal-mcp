"""
Blueprint Node Tools for Unreal MCP.

This module provides tools for manipulating Blueprint graph nodes and connections.
"""

import logging
from typing import Dict, List, Any
from mcp.server.fastmcp import FastMCP, Context
from utils.nodes.node_operations import (
    add_event_node as add_event_node_impl,
    add_input_action_node as add_input_action_node_impl,
    add_function_node as add_function_node_impl,
    connect_nodes_impl,
    add_variable as add_variable_impl,
    add_self_component_reference as add_self_component_reference_impl,
    add_self_reference as add_self_reference_impl,
    find_nodes as find_nodes_impl
)

# Get logger
logger = logging.getLogger("UnrealMCP")

def register_blueprint_node_tools(mcp: FastMCP):
    """Register Blueprint node manipulation tools with the MCP server."""
    
    @mcp.tool()
    def add_blueprint_event_node(
        ctx: Context,
        blueprint_name: str,
        event_name: str,
        node_position: List[float] = None
    ) -> Dict[str, Any]:
        """
        Add an event node to a Blueprint's event graph.
        
        Args:
            blueprint_name: Name of the target Blueprint
            event_name: Name of the event. Use 'Receive' prefix for standard events:
                       - 'ReceiveBeginPlay' for Begin Play
                       - 'ReceiveTick' for Tick
                       - etc.
            node_position: Optional [X, Y] position in the graph
            
        Returns:
            Response containing the node ID and success status
        """
        return add_event_node_impl(ctx, blueprint_name, event_name, node_position)
    
    @mcp.tool()
    def add_blueprint_input_action_node(
        ctx: Context,
        blueprint_name: str,
        action_name: str,
        node_position: List[float] = None
    ) -> Dict[str, Any]:
        """
        Add an input action event node to a Blueprint's event graph.
        
        Args:
            blueprint_name: Name of the target Blueprint
            action_name: Name of the input action to respond to
            node_position: Optional [X, Y] position in the graph
            
        Returns:
            Response containing the node ID and success status
        """
        return add_input_action_node_impl(ctx, blueprint_name, action_name, node_position)
    
    @mcp.tool()
    def add_blueprint_function_node(
        ctx: Context,
        blueprint_name: str,
        target: str,
        function_name: str,
        params: Dict[str, Any] = None,
        node_position: List[float] = None
    ) -> Dict[str, Any]:
        """
        Add a function call node to a Blueprint's event graph.
        
        Args:
            blueprint_name: Name of the target Blueprint
            target: Target object for the function (component name or self)
            function_name: Name of the function to call
            params: Optional parameters to set on the function node
            node_position: Optional [X, Y] position in the graph
            
        Returns:
            Response containing the node ID and success status
        """
        return add_function_node_impl(ctx, blueprint_name, target, function_name, params, node_position)
            
    @mcp.tool()
    def connect_blueprint_nodes(
        ctx: Context,
        blueprint_name: str,
        source_node_id: str,
        source_pin: str,
        target_node_id: str,
        target_pin: str
    ) -> Dict[str, Any]:
        """
        Connect two nodes in a Blueprint's event graph.
        
        Args:
            blueprint_name: Name of the target Blueprint
            source_node_id: ID of the source node
            source_pin: Name of the output pin on the source node
            target_node_id: ID of the target node
            target_pin: Name of the input pin on the target node
            
        Returns:
            Response indicating success or failure
        """
        return connect_nodes_impl(ctx, blueprint_name, source_node_id, source_pin, target_node_id, target_pin)
    
    @mcp.tool()
    def add_blueprint_variable(
        ctx: Context,
        blueprint_name: str,
        variable_name: str,
        variable_type: str,
        is_exposed: bool = False
    ) -> Dict[str, Any]:
        """
        Add a variable to a Blueprint.
        Supports built-in, user-defined struct, and delegate types.

        Args:
            blueprint_name: Name of the target Blueprint
            variable_name: Name of the variable
            variable_type: Type of the variable (Boolean, Integer, Float, Vector, StructName, StructName[], Delegate, etc.)
            is_exposed: Whether to expose the variable to the editor

        Returns:
            Response indicating success or failure

        Examples:
            # Add a basic integer variable
            add_blueprint_variable(
                ctx,
                blueprint_name="PlayerBlueprint",
                variable_name="Score",
                variable_type="Integer",
                is_exposed=True
            )

            # Add a string variable
            add_blueprint_variable(
                ctx,
                blueprint_name="PlayerBlueprint",
                variable_name="PlayerName",
                variable_type="String",
                is_exposed=True
            )

            # Add a vector variable
            add_blueprint_variable(
                ctx,
                blueprint_name="PlayerBlueprint",
                variable_name="Position",
                variable_type="Vector",
                is_exposed=True
            )

            # Add an array variable
            add_blueprint_variable(
                ctx,
                blueprint_name="PlayerBlueprint",
                variable_name="Inventory",
                variable_type="String[]",
                is_exposed=True
            )

            # Add a custom struct variable (using full path)
            add_blueprint_variable(
                ctx,
                blueprint_name="PlayerBlueprint",
                variable_name="Stats",
                variable_type="/Game/DataStructures/PlayerStats",
                is_exposed=True
            )
        """
        return add_variable_impl(ctx, blueprint_name, variable_name, variable_type, is_exposed)
    
    @mcp.tool()
    def add_blueprint_get_self_component_reference(
        ctx: Context,
        blueprint_name: str,
        component_name: str,
        node_position: List[float] = None
    ) -> Dict[str, Any]:
        """
        Add a node that gets a reference to a component owned by the current Blueprint.
        This creates a node similar to what you get when dragging a component from the Components panel.
        
        Args:
            blueprint_name: Name of the target Blueprint
            component_name: Name of the component to get a reference to
            node_position: Optional [X, Y] position in the graph
            
        Returns:
            Response containing the node ID and success status
        """
        return add_self_component_reference_impl(ctx, blueprint_name, component_name, node_position)
    
    @mcp.tool()
    def add_blueprint_self_reference(
        ctx: Context,
        blueprint_name: str,
        node_position: List[float] = None
    ) -> Dict[str, Any]:
        """
        Add a 'Get Self' node to a Blueprint's event graph that returns a reference to this actor.
        
        Args:
            blueprint_name: Name of the target Blueprint
            node_position: Optional [X, Y] position in the graph
            
        Returns:
            Response containing the node ID and success status
        """
        return add_self_reference_impl(ctx, blueprint_name, node_position)
    
    @mcp.tool()
    def find_blueprint_nodes(
        ctx: Context,
        blueprint_name: str,
        node_type: str = None,
        event_type: str = None
    ) -> Dict[str, Any]:
        """
        Find nodes in a Blueprint's event graph.
        
        Args:
            blueprint_name: Name of the target Blueprint
            node_type: Optional type of node to find (Event, Function, Variable, etc.)
            event_type: Optional specific event type to find (BeginPlay, Tick, etc.)
            
        Returns:
            Response containing array of found node IDs and success status
        """
        return find_nodes_impl(ctx, blueprint_name, node_type, event_type)
    
    logger.info("Blueprint node tools registered successfully")