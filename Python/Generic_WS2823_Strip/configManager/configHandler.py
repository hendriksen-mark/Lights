from configManager import configInit
from configManager.argumentHandler import parse_arguments
import os
import pathlib
import subprocess
import logManager
import yaml
import uuid
import weakref
try:
    from time import tzset
except ImportError:
    tzset = None

logging = logManager.logger.get_logger(__name__)

class NoAliasDumper(yaml.SafeDumper):
    def ignore_aliases(self, data):
        return True

def _open_yaml(path):
    with open(path, 'r', encoding="utf-8") as fp:
        return yaml.load(fp, Loader=yaml.FullLoader)

def _write_yaml(path, contents):
    with open(path, 'w', encoding="utf-8") as fp:
        yaml.dump(contents, fp , Dumper=NoAliasDumper, allow_unicode=True, sort_keys=False )


class Config:
    yaml_config = None
    configDir = parse_arguments()["CONFIG_PATH"]
    runningDir = str(pathlib.Path(__file__)).replace("/configManager/configHandler.py","")

    def __init__(self):
        if not os.path.exists(self.configDir):
            os.makedirs(self.configDir)

    def load_config(self):
        self.yaml_config = {"apiUsers": {}, "lights": {}, "groups": {}, "scenes": {}, "config": {}, "rules": {}, "resourcelinks": {}, "schedules": {}, "sensors": {}, "behavior_instance": {}, "geofence_clients": {}, "smart_scene": {}, "temp": {"eventstream": [], "scanResult": {"lastscan": "none"}, "detectedLights": [], "gradientStripLights": {}}}
        try:
            #load config
            if os.path.exists(self.configDir + "/config.yaml"):
                config = _open_yaml(self.configDir + "/config.yaml")
                

                self.yaml_config["config"] = config
            else:
                self.yaml_config["config"] = {
                }
            # load lights
            if os.path.exists(self.configDir + "/lights.yaml"):
                lights = _open_yaml(self.configDir + "/lights.yaml")
                for light, data in lights.items():
                    data["id_v1"] = light
                    self.yaml_config["lights"][light] = Light.Light(data)
                    #self.yaml_config["groups"]["0"].add_light(self.yaml_config["lights"][light])
            logging.info("Config loaded")
        except Exception:
            logging.exception("CRITICAL! Config file was not loaded")
            raise SystemExit("CRITICAL! Config file was not loaded")
        bridgeConfig = self.yaml_config

    def save_config(self, backup=False, resource="all"):
        path = self.configDir + '/'
        if backup:
            path = self.configDir + '/backup/'
            if not os.path.exists(path):
                os.makedirs(path)
        if resource in ["all", "config"]:
            config = self.yaml_config["config"]
            config["whitelist"] = {}
            for user, obj in self.yaml_config["apiUsers"].items():
                config["whitelist"][user] = obj.save()
            _write_yaml(path + "config.yaml", config)
            logging.debug("Dump config file " + path + "config.yaml")
            if resource == "config":
                return
        saveResources = []
        if resource == "all":
            saveResources = ["state"]
        else:
            saveResources.append(resource)
        for object in saveResources:
            filePath = path + object + ".yaml"
            dumpDict = {}
            for element in self.yaml_config[object]:
                if element != "0":
                    savedData = self.yaml_config[object][element].save()
                    if savedData:
                        dumpDict[self.yaml_config[object][element].id_v1] = savedData
            _write_yaml(filePath, dumpDict)
            logging.debug("Dump config file " + filePath)

    def reset_config(self):
        backup = self.save_config(backup=True)
        try:
            os.popen('rm -r ' + self.configDir + '/*.yaml')
        except:
            logging.exception("Something went wrong when deleting the config")
        self.load_config()
        return backup

    def restore_backup(self):
        try:
            os.popen('rm -r ' + self.configDir + '/*.yaml')
        except:
            logging.exception("Something went wrong when deleting the config")
        subprocess.run('cp -r ' + self.configDir + '/backup/*.yaml ' + self.configDir + '/', shell=True, capture_output=True, text=True)
        load = self.load_config()
        return load

    def download_config(self):
        self.save_config()
        subprocess.run('tar --exclude=' + "'config_debug.yaml'" + ' -cvf ' + self.configDir + '/config.tar ' + self.configDir + '/*.yaml', shell=True, capture_output=True, text=True)
        return self.configDir + "/config.tar"

    def download_log(self):
        subprocess.run('tar -cvf ' + self.configDir + '/diyhue_log.tar ' +
                 self.runningDir + '/*.log* ',
                 shell=True, capture_output=True, text=True)
        return self.configDir + "/diyhue_log.tar"

    def download_debug(self):
        _write_yaml(self.configDir + "/config_debug.yaml", self.yaml_config["config"])
        debug = _open_yaml(self.configDir + "/config_debug.yaml")
        debug["whitelist"] = "privately"
        debug["Hue Essentials key"] = "privately"
        debug["users"] = "privately"
        info = {}
        info["OS"] = os.uname().sysname
        info["Architecture"] = os.uname().machine
        info["os_version"] = os.uname().version
        info["os_release"] = os.uname().release
        info["Hue-Emulator Version"] = subprocess.run("stat -c %y HueEmulator3.py", shell=True, capture_output=True, text=True).stdout.replace("\n", "")
        info["WebUI Version"] = subprocess.run("stat -c %y flaskUI/templates/index.html", shell=True, capture_output=True, text=True).stdout.replace("\n", "")
        _write_yaml(self.configDir + "/config_debug.yaml", debug)
        _write_yaml(self.configDir + "/system_info.yaml", info)
        subprocess.run('tar --exclude=' + "'config.yaml'" + ' -cvf ' + self.configDir + '/config_debug.tar ' +
                 self.configDir + '/*.yaml ' +
                 self.runningDir + '/*.log* ',
                 shell=True, capture_output=True, text=True)
        os.popen('rm -r ' + self.configDir + '/config_debug.yaml')
        return self.configDir + "/config_debug.tar"

    def write_args(self, args):
        self.yaml_config = configInit.write_args(args, self.yaml_config)

    def generate_security_key(self):
        self.yaml_config = configInit.generate_security_key(self.yaml_config)
