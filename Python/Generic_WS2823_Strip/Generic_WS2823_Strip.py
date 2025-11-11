#!/usr/bin/env python
from flask import Flask
from flask_cors import CORS
from flask_restful import Api
import configManager
import logManager
from werkzeug.serving import WSGIRequestHandler
from flaskUI.restful import State, Detect, Config

lightConfig = configManager.lightConfig.yaml_config
logging = logManager.logger.get_logger(__name__)
_ = logManager.logger.get_logger("werkzeug")
WSGIRequestHandler.protocol_version = "HTTP/1.1"
app = Flask(__name__, template_folder='flaskUI/templates',static_url_path="/static", static_folder='flaskUI/static')
api = Api(app)
cors = CORS(app, resources={r"*": {"origins": "*"}})

app.config['SECRET_KEY'] = 'change_this_to_be_secure'
api.app.config['RESTFUL_JSON'] = {'ensure_ascii': False}

api.add_resource(State, '/state', strict_slashes=False)
api.add_resource(Detect, '/detect', strict_slashes=False)
api.add_resource(Config, '/config', strict_slashes=False)

### WEB INTERFACE
from flaskUI.core.views import core
from flaskUI.error_pages.handlers import error_pages

app.register_blueprint(core)
app.register_blueprint(error_pages)

def runHttp(BIND_IP, HOST_HTTP_PORT):
    app.run(host=BIND_IP, port=HOST_HTTP_PORT)

def main():
    HOST_IP = configManager.runtimeConfig.arg["HOST_IP"]
    HOST_HTTP_PORT = configManager.runtimeConfig.arg["HTTP_PORT"]
    CONFIG_PATH = configManager.runtimeConfig.arg["CONFIG_PATH"]
    runHttp(HOST_IP, HOST_HTTP_PORT)

if __name__ == '__main__':
    main()
