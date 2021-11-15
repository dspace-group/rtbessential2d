import os, platform
from datetime import datetime
from flask import (
    flash, g, redirect, render_template, request, session, url_for, make_response, jsonify, Flask
)
from pyrtb import Pyrtb

def create_app():
    # create and configure the app
    app = Flask(__name__, instance_relative_config=True, static_url_path='')

    # ensure the instance folder exists
    try:
        os.makedirs(app.instance_path)
    except OSError:
        pass
    
    path_to_lib = None
    if os.name == "posix":
        path_to_lib = "build/librtbessential2dctl.so"
    if os.name == "nt":
        path_to_lib = "build/Debug/rtbessential2dctl.dll"
    
    pyrtb = Pyrtb(path_to_lib)
    state = "Not initialized"
    logs = [{
        "time": datetime.now().strftime("%H:%M:%S"),
        "type": "✔",
        "msg": "Loaded"
    }]
    ecstates = ["Library ready", "Starting", "Started"]
    
    # a simple page that says hello
    @app.route('/')
    def root():
        return render_template('index.html')
    
    @app.route('/info')
    def info():
        info_obj = {}
        info_obj["machine"] = platform.machine()
        info_obj["system"] = platform.system()
        info_obj["state"] = state
        info_obj["ecstate"] = ecstates[pyrtb.Get_state()]
        info_obj["interfaces"] = pyrtb.Get_interfaces()
        info_obj["logs"] = logs[-5:]

        res = make_response(jsonify(info_obj), 200)
        return res
    
    @app.route('/initialize/<ifname>')
    def initialize(ifname=None):
        res = pyrtb.Start(ifname)
        if res == 0:
            state = "Initialized"
        elif res == 2:
            log = {
                "time": datetime.now().strftime("%H:%M:%S"),
                "type": "😕",
                "msg" : "Wrong amount of EtherCAT slaves found"
            }
            logs.append(log)
        elif res == 3:
            log = {
                "time": datetime.now().strftime("%H:%M:%S"),
                "type": "😕",
                "msg" : "Wrong EtherCAT slave found"
            }
            logs.append(log)            
        
        return render_template('base.html')

    return app