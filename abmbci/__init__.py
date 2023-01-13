import time
import logging
import asyncio
import threading
import abmbciext as abm

LOGGER = logging.getLogger(name=__name__)


def single_callback(chname, imp):
    LOGGER.debug("Signle Callback!")

class ABM:

    def __init__(self):
        self.started = False
        self.impedance = {}
        self._abm = abm
        self._device_info = abm.get_device_info_keep_connection()

    def __del__(self):
        self.stop()
        self._abm.close_current_connection()

    def setup_logging(self, level=logging.DEBUG):
        for name in logging.root.manager.loggerDict:
            logger = logging.getLogger(name)
            logger.setLevel(level)
        LOGGER.debug("Logging set")

    def set_config(self):
        self._abm.set_config_path()

    def init(self):
        val = self._abm.init_session_for_current_connection(1)
        if val == 0:
            LOGGER.warn("Init Session failed. Incorrect sequence.")
        elif val == -1:
            LOGGER.error("Failed to init session")
        elif val == 1:
            LOGGER.debug("Session Initialized")

    def start(self):
        if self.started:
            LOGGER.warn("Acquisition already started")
            return
        val = self._abm.start_acquisition_for_current_connection()
        if val == 1:
            self.started = True
            LOGGER.debug("Started")
        elif val == 0:
            LOGGER.warn("Failed to begin acquisition. Incorrect sequence.")
        elif val == -1:
            LOGGER.error("Failed to begin acquisition")

    def stop(self):
        if not self.started:
            LOGGER.warn("Acquisition not started. Nothing to stop")
            return
        val = self._abm.stop_acquisition_keep_connection()
        if val == 1:
            LOGGER.debug("Stopped")
            self.started = False
        elif val == 0:
            LOGGER.warn("IInappropriate sequence. Stopping ignored")
        elif val == -1:
            LOGGER.error("Failed to stop acquisition")

    def setup(self):
        self.set_config()
        self.init()
        self.start()          

    def get_data(self, n):
        return self._abm.get_raw_data(self._device_info, n)

    def set_imp_callback(self, callback=None):
        if callback is None:
            callback = lambda n, i: self.impedance.update({n: i})
        val = self._abm.register_callback_impedance_electrode_finished_a(callback)
        if val == 0:
            LOGGER.error("Failed to register callback")
            return
        elif val == 1:
            LOGGER.debug("callback set")

    def check_imp(self, cb=lambda x, i: 0, electrodes=["C3", "C4", "Cz"]):
        self.impedance = {}
        val = self._abm.check_selected_impedances_for_current_connection(cb, electrodes)
        if val == 1:
            LOGGER.debug("Impedance check started")
        elif val == 0:
            LOGGER.warn("Invalid squence. Impedance check ignored")
            return
        elif val == -1:
            LOGGER.error("Failed to start impedance check")
            return

    def cb_and_imp(self):
        self.set_imp_callback()
        self.check_imp()

    def full_imp(self):
        self.init()
        self.check_imp()

    def af(self):
        self.event = threading.Event()
        self.event.clear()

        self.init()
        self.set_imp_callback()
        def final_callback(x, i):
            print(f"Finally Finished on thread {threading.get_native_id()}")
            self.event.set()
            print(f"Set: {self.event.is_set()}")

        self.check_imp(final_callback)
        print(f"Checking initiated on thread: {threading.get_native_id()}")
        self.event.wait(timeout=120.0)