import configManager
import logManager

logging = logManager.logger.get_logger(__name__)

lightConfig = configManager.lightConfig.yaml_config

class LightEngine:
    def light_engine(self):
        # Core light processing logic
        # This function will be called in a loop to process light data
        while True:
            # Simulate light processing
            for light in lightConfig["lights"].values():
                self.process_lightdata(light)
            time.sleep(0.006)