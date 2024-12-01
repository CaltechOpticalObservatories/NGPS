class InstrumentStatusModel:
    def __init__(self):
        self.camera_status = "Not covered"
        self.exposure_time = "1000 ms"
        self.slit_value = "0.5 mm"
        self.temperature = "22°C"
        self.bin_value = "2"

    def set_camera_status(self, status):
        self.camera_status = status

    def set_exposure_time(self, time):
        self.exposure_time = time

    def set_slit_value(self, value):
        self.slit_value = value

    def set_temperature(self, temp):
        self.temperature = temp

    def set_bin_value(self, bin_value):
        self.bin_value = bin_value

