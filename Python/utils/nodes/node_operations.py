"""
Node Operations for Unreal MCP.

This module provides utilities for working with Blueprint nodes in Unreal Engine.
"""

import logging
from typing import Dict, List, Any
from mcp.server.fastmcp import Context
from utils.unreal_connection_utils import send_unreal_command

# Get logger
logger = logging.getLogger("UnrealMCP")

def add_node(
    ctx: Context,
    blueprint_path: str,
    function_name: str,
    node_type: str,
    node_name: str,
    x: float,
    y: float,
    inputs: Dict[str, Any] = None,
    outputs: Dict[str, Any] = None
) -> Dict[str, Any]:
    """Implementation for adding a node to a blueprint graph."""
    if inputs is None:
        inputs = {}
    if outputs is None:
        outputs = {}
    
    params = {
        "blueprint_path": blueprint_path,
        "function_name": function_name,
        "node_type": node_type,
        "node_name": node_name,
        "x": x,
        "y": y,
        "inputs": inputs,
        "outputs": outputs
    }
    
    return send_unreal_command("add_node", params)

def connect_nodes(
    ctx: Context,
    blueprint_path: str,
    function_name: str,
    source_node_name: str,
    source_pin_name: str,
    target_node_name: str,
    target_pin_name: str
) -> Dict[str, Any]:
    """Implementation for connecting two nodes in a blueprint graph."""
    params = {
        "blueprint_path": blueprint_path,
        "function_name": function_name,
        "source_node_name": source_node_name,
        "source_pin_name": source_pin_name,
        "target_node_name": target_node_name,
        "target_pin_name": target_pin_name
    }
    
    return send_unreal_command("connect_blueprint_nodes", params)

def add_variable_get(
    ctx: Context,
    blueprint_path: str,
    function_name: str,
    variable_name: str,
    x: float,
    y: float
) -> Dict[str, Any]:
    """Implementation for adding a variable get node to a blueprint graph."""
    params = {
        "blueprint_path": blueprint_path,
        "function_name": function_name,
        "variable_name": variable_name,
        "x": x,
        "y": y
    }
    
    return send_unreal_command("add_variable_get", params)

def add_variable_set(
    ctx: Context,
    blueprint_path: str,
    function_name: str,
    variable_name: str,
    x: float,
    y: float,
    value: Any = None
) -> Dict[str, Any]:
    """Implementation for adding a variable set node to a blueprint graph."""
    params = {
        "blueprint_path": blueprint_path,
        "function_name": function_name,
        "variable_name": variable_name,
        "x": x,
        "y": y
    }
    
    if value is not None:
        params["value"] = value
    
    return send_unreal_command("add_variable_set", params)

def add_event(
    ctx: Context,
    blueprint_path: str,
    event_type: str,
    event_name: str = None,
    x: float = 0.0,
    y: float = 0.0
) -> Dict[str, Any]:
    """Implementation for adding an event node to a blueprint graph."""
    params = {
        "blueprint_path": blueprint_path,
        "event_type": event_type,
        "x": x,
        "y": y
    }
    
    if event_name is not None:
        params["event_name"] = event_name
    
    return send_unreal_command("add_event", params)

def add_custom_event(
    ctx: Context,
    blueprint_path: str,
    event_name: str,
    inputs: List[Dict[str, Any]] = None,
    outputs: List[Dict[str, Any]] = None,
    x: float = 0.0,
    y: float = 0.0
) -> Dict[str, Any]:
    """Implementation for adding a custom event node to a blueprint graph."""
    if inputs is None:
        inputs = []
    if outputs is None:
        outputs = []
        
    params = {
        "blueprint_path": blueprint_path,
        "event_name": event_name,
        "inputs": inputs,
        "outputs": outputs,
        "x": x,
        "y": y
    }
    
    return send_unreal_command("add_custom_event", params)

def add_branch(
    ctx: Context,
    blueprint_path: str,
    function_name: str,
    condition: bool = None,
    x: float = 0.0,
    y: float = 0.0
) -> Dict[str, Any]:
    """Implementation for adding a branch node (if statement) to a blueprint graph."""
    params = {
        "blueprint_path": blueprint_path,
        "function_name": function_name,
        "x": x,
        "y": y
    }
    
    if condition is not None:
        params["condition"] = condition
    
    return send_unreal_command("add_branch", params)

def add_sequence(
    ctx: Context,
    blueprint_path: str,
    function_name: str,
    num_pins: int = 2,
    x: float = 0.0,
    y: float = 0.0
) -> Dict[str, Any]:
    """Implementation for adding a sequence node to a blueprint graph."""
    params = {
        "blueprint_path": blueprint_path,
        "function_name": function_name,
        "num_pins": num_pins,
        "x": x,
        "y": y
    }
    
    return send_unreal_command("add_sequence", params)

def add_function_call(
    ctx: Context,
    blueprint_path: str,
    function_name: str,
    target_function_path: str,
    x: float = 0.0,
    y: float = 0.0,
    input_values: Dict[str, Any] = None
) -> Dict[str, Any]:
    """Implementation for adding a function call node to a blueprint graph."""
    params = {
        "blueprint_path": blueprint_path,
        "function_name": function_name,
        "target_function_path": target_function_path,
        "x": x,
        "y": y
    }
    
    if input_values is not None:
        params["input_values"] = input_values
    
    return send_unreal_command("add_function_call", params)

def add_comment(
    ctx: Context,
    blueprint_path: str,
    function_name: str,
    comment_text: str,
    x: float = 0.0,
    y: float = 0.0,
    width: float = 300.0,
    height: float = 200.0,
    color: List[float] = [1.0, 1.0, 1.0, 1.0]
) -> Dict[str, Any]:
    """Implementation for adding a comment node to a blueprint graph."""
    params = {
        "blueprint_path": blueprint_path,
        "function_name": function_name,
        "comment_text": comment_text,
        "x": x,
        "y": y,
        "width": width,
        "height": height,
        "color": color
    }
    
    return send_unreal_command("add_comment", params)

def get_all_nodes(
    ctx: Context,
    blueprint_path: str,
    function_name: str
) -> List[Dict[str, Any]]:
    """Implementation for getting information about all nodes in a blueprint graph."""
    params = {
        "blueprint_path": blueprint_path,
        "function_name": function_name
    }
    
    return send_unreal_command("get_all_nodes", params)

def get_node_details(
    ctx: Context,
    blueprint_path: str,
    function_name: str,
    node_name: str
) -> Dict[str, Any]:
    """Implementation for getting detailed information about a specific node in a blueprint graph."""
    params = {
        "blueprint_path": blueprint_path,
        "function_name": function_name,
        "node_name": node_name
    }
    
    return send_unreal_command("get_node_details", params)

def add_event_node(
    ctx: Context,
    blueprint_name: str,
    event_name: str,
    node_position: List[float] = None
) -> Dict[str, Any]:
    """Implementation for adding an event node to a Blueprint's event graph."""
    params = {
        "blueprint_name": blueprint_name,
        "event_name": event_name
    }
    
    if node_position is not None:
        params["node_position"] = node_position
        
    return send_unreal_command("add_blueprint_event_node", params)

def add_input_action_node(
    ctx: Context,
    blueprint_name: str,
    action_name: str,
    node_position: List[float] = None
) -> Dict[str, Any]:
    """Implementation for adding an input action event node to a Blueprint's event graph."""
    params = {
        "blueprint_name": blueprint_name,
        "action_name": action_name
    }
    
    if node_position is not None:
        params["node_position"] = node_position
        
    return send_unreal_command("add_blueprint_input_action_node", params)

def add_function_node(
    ctx: Context,
    blueprint_name: str,
    target: str,
    function_name: str,
    params: Dict[str, Any] = None,
    node_position: List[float] = None
) -> Dict[str, Any]:
    """Implementation for adding a function call node to a Blueprint's event graph."""
    command_params = {
        "blueprint_name": blueprint_name,
        "target": target,
        "function_name": function_name
    }
    
    if params is not None:
        command_params["params"] = params
        
    if node_position is not None:
        command_params["node_position"] = node_position
        
    return send_unreal_command("add_blueprint_function_node", command_params)

def add_variable(
    ctx: Context,
    blueprint_name: str,
    variable_name: str,
    variable_type: str,
    is_exposed: bool = False
) -> Dict[str, Any]:
    """Implementation for adding a variable to a Blueprint.
    Supports built-in, user-defined struct, and delegate types.
    """
    params = {
        "blueprint_name": blueprint_name,
        "variable_name": variable_name,
        "variable_type": variable_type,
        "is_exposed": is_exposed
    }
    
    return send_unreal_command("add_blueprint_variable", params)

def add_self_component_reference(
    ctx: Context,
    blueprint_name: str,
    component_name: str,
    node_position: List[float] = None
) -> Dict[str, Any]:
    """Implementation for adding a node that gets a reference to a component owned by the current Blueprint."""
    params = {
        "blueprint_name": blueprint_name,
        "component_name": component_name
    }
    
    if node_position is not None:
        params["node_position"] = node_position
        
    return send_unreal_command("add_blueprint_get_self_component_reference", params)

def add_self_reference(
    ctx: Context,
    blueprint_name: str,
    node_position: List[float] = None
) -> Dict[str, Any]:
    """Implementation for adding a 'Get Self' node to a Blueprint's event graph."""
    params = {
        "blueprint_name": blueprint_name
    }
    
    if node_position is not None:
        params["node_position"] = node_position
        
    return send_unreal_command("add_blueprint_self_reference", params)

def find_nodes(
    ctx: Context,
    blueprint_name: str,
    node_type: str = None,
    event_type: str = None
) -> Dict[str, Any]:
    """Implementation for finding nodes in a Blueprint's event graph."""
    params = {
        "blueprint_name": blueprint_name,
        "node_type": node_type if node_type is not None else "All"
    }
    
    if event_type is not None:
        params["event_name"] = event_type
        
    return send_unreal_command("find_blueprint_nodes", params)

def connect_nodes_impl(
    ctx: Context,
    blueprint_name: str,
    source_node_id: str,
    source_pin: str,
    target_node_id: str,
    target_pin: str
) -> Dict[str, Any]:
    """Implementation for connecting two nodes in a Blueprint's event graph."""
    params = {
        "blueprint_name": blueprint_name,
        "source_node_id": source_node_id,
        "source_pin": source_pin,
        "target_node_id": target_node_id,
        "target_pin": target_pin
    }
    
    return send_unreal_command("connect_blueprint_nodes", params)