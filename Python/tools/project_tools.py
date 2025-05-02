"""
Project Tools for Unreal MCP.

This module provides tools for managing project-wide settings and configuration.
"""

import logging
from typing import Dict, Any, List
from mcp.server.fastmcp import FastMCP, Context
from utils.project.struct_operations import create_struct as create_struct_impl
from utils.project.struct_operations import update_struct as update_struct_impl
from utils.project.struct_operations import show_struct_variables as show_struct_variables_impl
from utils.project.struct_operations import list_folder_contents as list_folder_contents_impl

# Get logger
logger = logging.getLogger("UnrealMCP")

def register_project_tools(mcp: FastMCP):
    """Register project tools with the MCP server."""
    
    @mcp.tool()
    def create_input_mapping(
        ctx: Context,
        action_name: str,
        key: str,
        input_type: str = "Action"
    ) -> Dict[str, Any]:
        """
        Create an input mapping for the project.
        
        Args:
            action_name: Name of the input action
            key: Key to bind (SpaceBar, LeftMouseButton, etc.)
            input_type: Type of input mapping (Action or Axis)
            
        Returns:
            Response indicating success or failure
        
        Example:
            create_input_mapping(action_name="Jump", key="SpaceBar")
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            
            params = {
                "action_name": action_name,
                "key": key,
                "input_type": input_type
            }
            
            logger.info(f"Creating input mapping '{action_name}' with key '{key}'")
            response = unreal.send_command("create_input_mapping", params)
            
            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}
            
            logger.info(f"Input mapping creation response: {response}")
            return response
            
        except Exception as e:
            error_msg = f"Error creating input mapping: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}
    
    @mcp.tool()
    def create_folder(
        ctx: Context,
        folder_path: str
    ) -> Dict[str, Any]:
        """
        Create a new folder in the Unreal project.

        This tool can create both content folders (which will appear in the content browser)
        and regular project folders.

        Args:
            folder_path: Path to create, relative to project root.
                       Use "Content/..." prefix for content browser folders.

        Returns:
            Dictionary with the creation status and folder path
            
        Examples:
            # Create a content browser folder
            create_folder(folder_path="Content/MyGameContent")
            
            # Create a regular project folder
            create_folder(folder_path="Intermediate/MyTools")
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            
            params = {
                "folder_path": folder_path
            }
            
            logger.info(f"Creating folder: {folder_path}")
            response = unreal.send_command("create_folder", params)
            
            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}
            
            logger.info(f"Folder creation response: {response}")
            return response
            
        except Exception as e:
            error_msg = f"Error creating folder: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}
    
    @mcp.tool()
    def create_struct(
        ctx: Context,
        struct_name: str,
        properties: List[Dict[str, str]],
        path: str = "/Game/Blueprints",
        description: str = ""
    ) -> Dict[str, Any]:
        """
        Create a new Unreal struct.
        
        Args:
            struct_name: Name of the struct to create
            properties: List of property dictionaries, each containing:
                        - name: Property name
                        - type: Property type (e.g., "Boolean", "Integer", "Float", "String", "Vector", etc.)
                        - description: (Optional) Property description
            path: Path where to create the struct
            description: Optional description for the struct
            
        Returns:
            Dictionary with the creation status and struct path
            
        Examples:
            # Create a simple Item struct
            create_struct(
                struct_name="Item",
                properties=[
                    {"name": "Name", "type": "String"},
                    {"name": "Value", "type": "Integer"},
                    {"name": "IsRare", "type": "Boolean"}
                ],
                path="/Game/DataStructures"
            )
        """
        return create_struct_impl(ctx, struct_name, properties, path, description)

    @mcp.tool()
    def update_struct(
        ctx: Context,
        struct_name: str,
        properties: List[Dict[str, str]],
        path: str = "/Game/Blueprints",
        description: str = ""
    ) -> Dict[str, Any]:
        """
        Update an existing Unreal struct.
        Args:
            struct_name: Name of the struct to update
            properties: List of property dictionaries, each containing:
                        - name: Property name
                        - type: Property type (e.g., "Boolean", "Integer", "Float", "String", "Vector", etc.)
                        - description: (Optional) Property description
            path: Path where the struct exists
            description: Optional description for the struct
        Returns:
            Dictionary with the update status and struct path
        """
        return update_struct_impl(ctx, struct_name, properties, path, description)

    @mcp.tool()
    def show_struct_variables(
        ctx: Context,
        struct_name: str,
        path: str = "/Game/Blueprints"
    ) -> Dict[str, Any]:
        """
        Show variables and types of a struct in Unreal Engine.
        Args:
            struct_name: Name of the struct to inspect
            path: Path where the struct exists (default: /Game/Blueprints)
        Returns:
            Dictionary with struct variable info
        """
        return show_struct_variables_impl(ctx, struct_name, path)

    @mcp.tool()
    def list_folder_contents(
        ctx: Context,
        folder_path: str
    ) -> Dict[str, Any]:
        """
        List the contents of a folder in the Unreal project (content or regular folder).
        Args:
            folder_path: Path to the folder (e.g., "/Game/Blueprints" or "Content/MyFolder" or "Intermediate/MyTools")
        Returns:
            Dictionary with arrays of subfolders and files/assets
        Example:
            list_folder_contents(folder_path="/Game/Blueprints")
        """
        return list_folder_contents_impl(ctx, folder_path)

    logger.info("Project tools registered successfully")