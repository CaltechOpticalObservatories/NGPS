#!/usr/bin/python3.9

import json
import os
import sys
import subprocess
import zmq
from threading import Thread
from kivy.app import App
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.button import Button
from kivy.clock import Clock
from kivy.config import Config
from kivy.uix.label import Label
from kivy.graphics import Color, Rectangle

Config.set('graphics', 'width', '220')
Config.set('graphics', 'height', '220')

NGPS_ROOT = os.environ.get("NGPS_ROOT", "/home/developer/Software")

class MyBoxLayout(BoxLayout):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        with self.canvas.before:
            # Default background color (black)
            self.bg_color = Color(0, 0, 0, 1)
            self.rect = Rectangle(size=self.size, pos=self.pos)
        self.bind(size=self.update_rect, pos=self.update_rect)

    def update_rect(self, *args):
        self.rect.pos = self.pos
        self.rect.size = self.size

    def set_background_color(self, color):
        self.bg_color.rgba = color

class TargetControlApp(App):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.button = Button(text="standby",
                             background_color=[1, 1, 0, 1],
                             size_hint=(1,None),
                             height=100,
                             disabled=True)
        self.cancelbutton = Button(text="override wait for slew",
                                   size_hint=(1, None),
                                   background_color=[0.5, 0.5, 0.5, 1],
                                   height=25,
                                   opacity=0)
        self.command = f\"{NGPS_ROOT}/run/seq ontarget\"
        self.button.bind(on_press=self.on_button_click)
        self.cancelbutton.bind(on_press=self.on_button_cancel_wait)
        self.button.bind(disabled=self.on_button_disabled)

        self.button.text_size = self.button.size
        self.button.halign = 'center'
        self.button.valign = 'middle'

        self.status_label_top = Label(text="", size_hint=(1, 0.2), pos_hint={'top': 0.9})
        self.status_label_bottom = Label(text="", size_hint=(1, 0.2), pos_hint={'top': 0.1})
        
        self.layout = MyBoxLayout(orientation='vertical', padding=30)
        self.layout.add_widget(self.status_label_top)
        self.layout.add_widget(self.button)
        self.layout.add_widget(self.cancelbutton)
        self.layout.add_widget(self.status_label_bottom)

        self.root = self.layout
        self.is_target = False
        self.flashing_event = None
        self.cancel_wait = False
        self.block_tcsop = False
        self.bellcounter=0

        # setup ZMQ subscriber
        self.context = zmq.Context()
        self.subscriber = self.context.socket(zmq.SUB)
        self.subscriber.connect("tcp://localhost:5556")
        self.subscriber.setsockopt_string(zmq.SUBSCRIBE, "seq_waitstate")

        # start ZMQ listening thread
        self.subscriber_thread = Thread(target=self.listen_to_zmq)
        self.subscriber_thread.daemon = True
        self.subscriber_thread.start()

    def on_button_click(self, instance):
        if not self.is_target:
            self.button.background_color = [0, 1, 0, 1]
            subprocess.Popen(self.command, shell=True)
            self.is_target = True
            self.cancel_wait = False
            self.button.disabled = True
            self.block_tcsop = True
            Clock.schedule_once(lambda dt: setattr(self, 'block_tcsop', False), 3)

    def on_button_disabled(self, instance, value):
        if value:
            self.button.text="standby"
        else:
            self.button.text="Click When\nTarget Ready"

    def on_button_cancel_wait(self, instance):
        self.button.background_color = [1, 1, 0, 1]
        self.is_target = False
        self.cancel_wait = True
        self.stop_flashing()

    def check_status(self, dt=10):
        process = subprocess.Popen("seq state", shell=True, stdout=subprocess.PIPE)
        output, _ = process.communicate()
        output = output.decode().strip()
        self.button.background_color = [0, 1, 0, 1]
        if 'TCSOP' in output.lower():
            self.button.background_color = [1, 1, 0, 1]
            self.is_target = False
        Clock.schedule_once(self.check_status, dt)

    def listen_to_zmq(self):
        while True:
            try:
                # Receive the message
                topic, payload = self.subscriber.recv_multipart()
                topic = topic.decode()
                payload = payload.decode()

                # Ensure the message starts with the expected topic
                if topic != "seq_waitstate":
                    continue

#               if self.block_tcsop:
#                   continue

                data = json.loads(payload)

                if data.get("TCSOP"):
                    if not self.cancel_wait:
                        self.start_flashing()

                # Log raw message for debugging
                # print(f'Received topic="{topic}" payload={payload}')
            
            except Exception as e:
                print(f"Error in listen_to_zmq: {e}")

    def play_system_bell(self):
        # Print the ASCII bell character to trigger the system bell
        sys.stdout.write('\a')  # This triggers the system bell
        sys.stdout.flush()

    def start_flashing(self):
        self.is_target = False
        self.button.disabled = True
        if self.flashing_event is None:
            self.flashing_event = Clock.schedule_interval(self.flash_background, 0.5)

    def flash_background(self, dt):
        # If the background is black, change to red and make the text black
        if self.layout.bg_color.rgba == [0, 0, 0, 1]:  # Black background
            self.layout.set_background_color([1, 0, 0, 1])  # Red background
            self.status_label_top.text = "waiting for slew"
            self.status_label_bottom.text = "waiting for slew"
            self.status_label_top.color = [0, 0, 0, 1]  # Black text on red background
            self.status_label_bottom.color = [0, 0, 0, 1]  # Black text on red background
            if self.bellcounter >= 3:
                self.play_system_bell()
                self.cancelbutton.opacity=1
            self.bellcounter += 1
        else:  # If the background is red, change to black and make the text red
            self.layout.set_background_color([0, 0, 0, 1])  # Black background
            self.status_label_top.text = "waiting for slew"
            self.status_label_bottom.text = "waiting for slew"
            self.status_label_top.color = [1, 0, 0, 1]  # Red text on black background
            self.status_label_bottom.color = [1, 0, 0, 1]  # Red text on black background

        # Send the shell command and check the response
        process = subprocess.Popen(f\"{NGPS_ROOT}/run/tcs getmotion\", shell=True, stdout=subprocess.PIPE)
        output, _ = process.communicate()
        output = output.decode().strip()

        if "DONE" in output and "tracking" not in output:
            self.stop_flashing()

    def stop_flashing(self):
        self.bellcounter=0
        self.cancelbutton.opacity=0
        self.button.disabled = False  # Make it clickable
        if self.flashing_event:
            Clock.unschedule(self.flashing_event)
            self.flashing_event = None
            self.layout.set_background_color([0, 0, 0, 1])  # Black background
            self.status_label_top.text = ""
            self.status_label_bottom.text = ""
            self.status_label_top.color = [1, 0, 0, 1]  # Red text
            self.status_label_bottom.color = [1, 0, 0, 1]  # Red text

    def build(self):
        return self.root

if __name__ == '__main__':
    TargetControlApp().run()
