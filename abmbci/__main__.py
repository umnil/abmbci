import os
import time
import logging
import threading
import abmbci

from abmbci import abm
from datetime import datetime, timedelta


logging.basicConfig(level=logging.DEBUG)


def detection_device_callback(message, i):
    print(f"Device Callback: {message}")

def status_info_callback(status_info):
    print(f"Battery Level: %{status_info.battery_percentage}")

def main():
    impedances = {}
    val = input(f"HI {os.getpid()}: ")
    print("Starting")
    abm.set_log_path(os.path.join(os.getcwd(), "log"))
    log_path = abm.get_log_path()
    print(f"Creating log path: {log_path}")
    abm.ensure_log_path()

    print("Registering Callback")
    print(f"Python Thread ID {threading.get_native_id()}")
    abm.register_callback_device_detection_info(detection_device_callback)
    abm.register_callback_on_status_info(status_info_callback)

    device_info = abm.get_device_info_keep_connection()
    if device_info is None:
        print("Failed to find device")
        abm.close_current_connection()
        print("Done")
        exit(1)

    print(f"Found Device: {device_info.device_name}")

    result = abm.set_config_path()
    if result != 1:
        print("Failed to set config path")
        abm.close_current_connection()
        exit(1)

    result = abm.init_session_for_current_connection(1)
    if result != 1:
        print("Failed to initialize EEG session")
        abm.close_current_connection()
        exit(1)

    def cb(x, i):
        print(f"Impedance for channel {x}: {i}")
        impedances.update({x: i})

    result = abm.register_callback_impedance_electrode_finished_a(cb)
    if result != 1:
        print("Failed to register electrode impedance callback")
        abm.close_current_connection()
        exit(1)

    print("Callback registered")

    electrode_list = ["C3", "Cz", "C4"]
    result = abm.check_selected_impedances_for_current_connection(lambda e,i: print("Done"), electrode_list)
    if result == 0:
        print("Impedance checking request was ignored")
    elif result == -1:
        print("Failed to start impedance checking")
    elif result == 1:
        print("Impedance checking started")

    print("Waiting for callbacks")
    while len(impedances) < 4:
        time.sleep(0.5)
    # for t in threading.enumerate():
    #     print(f"Thread: {t.native_id}")
    # start_time = datetime.now()
    # cur_time = datetime.now()
    # time_limit = timedelta(seconds=12)
    # diff_time = cur_time - start_time
    # while diff_time < time_limit:
    #     diff_time = (datetime.now() - start_time)

    # print(len(impedances))

    print("Shutting down connection")
    abm.close_current_connection()
    print("Done")

if __name__ == "__main__":
    # asyncio.run(main())
    main()