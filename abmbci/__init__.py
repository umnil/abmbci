import abmbciext as abm


class ABM:

    def __init__(self):
        self._abm = abm
        self._device_info = abm.get_device_info_keep_connection()

    def __del__(self):
        self._abm.stop_acquisition_keep_connection()
        self._abm.close_current_connection()

    def setup(self):
        self._abm.set_config_path()
        self._abm.init_session_for_current_connection(1)
        self._abm.start_acquisition_for_current_connection()

    def get_data(self, n):
        return self._abm.get_raw_data(self._device_info, n)