from configManager import configHandler
from configManager import argumentHandler
from configManager import runtimeConfigHandler

lightConfig = configHandler.Config()
runtimeConfig = runtimeConfigHandler.Config()

# Initialize runtime configuration
runtimeConfig.populate()
argumentHandler.process_arguments(lightConfig.configDir, runtimeConfig.arg)

# Restore configuration
lightConfig.load_config()

# Initialize bridge config
#lightConfig.generate_security_key()
lightConfig.write_args(runtimeConfig.arg)
