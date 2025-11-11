from flask_restful import Resource
from flask import request
import configManager
import logManager

logging = logManager.logger.get_logger(__name__)

lightConfig = configManager.lightConfig.yaml_config


class State(Resource):
    def get(self):

        # Dynamically fetch the light index from request arguments
        light_index = int(request.args.get("light", 1)) - 1  # Default to light 1 if not provided

        # Example light object (replace with actual light data structure)
        lights = [{
            "lightState": True,
            "bri": 200,
            "x": 0.5,
            "y": 0.5,
            "ct": 250,
            "hue": 10000,
            "sat": 200,
            "colorMode": 1
        }]

        # Ensure the light index is within bounds
        if light_index < 0 or light_index >= len(lights):
            return {"error": "Invalid light index"}, 400

        light = lights[light_index]

        # Construct the response
        response = {
            "on": light["lightState"],
            "bri": light["bri"],
            "xy": [light["x"], light["y"]],
            "ct": light["ct"],
            "hue": light["hue"],
            "sat": light["sat"],
        }

        # Add colormode based on colorMode value
        if light["colorMode"] == 1:
            response["colormode"] = "xy"
        elif light["colorMode"] == 2:
            response["colormode"] = "ct"
        elif light["colorMode"] == 3:
            response["colormode"] = "hs"

        return response

    def put(self):
        # Parse incoming JSON data
        values = request.get_json()
        if not values:
            return {"error": "Invalid JSON payload"}, 400

        state_save = False
        transitiontime = 4  # Default transition time

        # Example light object (replace with actual light data structure)
        lights = [{
            "x": 0.0,
            "y": 0.0,
            "ct": 0,
            "hue": 0,
            "sat": 0,
            "colorMode": 0,
            "lightState": False,
            "bri": 0,
            "currentColors": [0, 0, 0]
        }]

        for key, light_values in values.items():
            light = int(key) - 1  # Convert key to light index

            # Ensure the light index is within bounds
            if light < 0 or light >= len(lights):
                return {"error": f"Invalid light index: {light + 1}"}, 400

            # Update xy values
            if "xy" in light_values:
                lights[light]["x"] = light_values["xy"][0]
                lights[light]["y"] = light_values["xy"][1]
                lights[light]["colorMode"] = 1
            # Update ct value
            elif "ct" in light_values:
                lights[light]["ct"] = light_values["ct"]
                lights[light]["colorMode"] = 2
            # Update hue and sat values
            else:
                if "hue" in light_values:
                    lights[light]["hue"] = light_values["hue"]
                    lights[light]["colorMode"] = 3
                if "sat" in light_values:
                    lights[light]["sat"] = light_values["sat"]
                    lights[light]["colorMode"] = 3

            # Update on/off state
            if "on" in light_values:
                lights[light]["lightState"] = bool(light_values["on"])
                state_save = True

            # Update brightness
            if "bri" in light_values:
                lights[light]["bri"] = light_values["bri"]

            # Increment brightness
            if "bri_inc" in light_values:
                lights[light]["bri"] += int(light_values["bri_inc"])
                if lights[light]["bri"] > 255:
                    lights[light]["bri"] = 255
                elif lights[light]["bri"] < 1:
                    lights[light]["bri"] = 1

            # Update transition time
            if "transitiontime" in light_values:
                transitiontime = light_values["transitiontime"]

            # Handle alert
            if "alert" in light_values and light_values["alert"] == "select":
                if lights[light]["lightState"]:
                    lights[light]["currentColors"] = [0, 0, 0]
                else:
                    lights[light]["currentColors"][1] = 126
                    lights[light]["currentColors"][2] = 126

            # Process light data (replace with actual processing function)
            processLightdata(light, transitiontime)

        # Save state if required
        if state_save:
            saveState()  # Replace with actual state-saving function

        return {"status": "success", "message": "Light state updated"}
    
class Detect(Resource):
    def get(self):
        return {
            "name": "success",
            "lights": "Detection started",
            "protocol": "Detection started",
            "modelid": "Detection in progress",
            "type": bridgeConfig,
            "mac": "hardware_info",
            "version": "software_info",
        }

class Config(Resource):
    def get(self):
        response = {
            "name": "success",
            "scene": "Configuration retrieved",
            "startup": bridgeConfig,
            "hw": "hardware_info",
            "on": "software_info",
            "off": bridgeConfig,
            "hwswitch": bridgeConfig["version"],
            "lightscount": bridgeConfig["author"],
            "pixelcount": bridgeConfig["author"],
            "transitionleds": bridgeConfig["description"],
            "rpct": bridgeConfig["description"],
            "gpct": bridgeConfig["version"],
            "bpct": bridgeConfig["author"],
            "disdhcp": bridgeConfig["description"],
            "addr": bridgeConfig["version"],
            "gw": bridgeConfig["author"],
            "sm": bridgeConfig["description"]
        }
        
        # Dynamically add dividedLight_<index> keys
        dividedLightsArray = [1, 2, 3]  # Replace with actual array
        for i, value in enumerate(dividedLightsArray):
            response[f"dividedLight_{i}"] = int(value)
        
        return response