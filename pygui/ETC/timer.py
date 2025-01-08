# timer.py

import time

class TimerError(Exception):
    """A custom exception used to report errors in use of Timer class"""

class Timer:
    def __init__(self, msg='Timer' ,turnOn=True):
        self._start_time = None
        self.msg=msg
        self.turnOn = turnOn

    def start(self):
        """Start a new timer"""
        if self._start_time is not None:
            raise TimerError(f"Timer is running. Use .stop() to stop it")

        self._start_time = time.perf_counter()

    def stop(self ,tag=None):
        """Stop the timer, and report the elapsed time"""
        if self._start_time is None:
            raise TimerError(f"Timer is not running. Use .start() to start it")

        elapsed_time = time.perf_counter() - self._start_time
        self._start_time = None
        msg = self.msg
        if tag is None: print(f"{msg}: {elapsed_time} seconds")
        else: print(f"{tag}: {elapsed_time} seconds")

    def __enter__(self):
        """Start a new timer as a context manager"""
        if self.turnOn: self.start()
        return self

    def __exit__(self, *exc_info):
        """Stop the context manager timer"""
        if self.turnOn: self.stop()        
