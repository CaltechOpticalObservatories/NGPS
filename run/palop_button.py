#!/usr/bin/python3.9

# Widget with 1 button to send a sequencer command

import tkinter as tk
import os

def button_clicked():
    cmd = "seq ontarget"
    print(cmd)
    os.system(cmd)

root = tk.Tk()

# Creating a button with specified options
button = tk.Button(root, 
                   text="On Target", 
                   command=button_clicked,
                   activebackground="blue", 
                   activeforeground="white",
                   anchor="center",
                   bd=3,
                   bg="lightgray",
                   cursor="hand2",
                   disabledforeground="gray",
                   fg="black",
                   font=("Arial", 12),
                   height=2,
                   highlightbackground="black",
                   highlightcolor="green",
                   highlightthickness=2,
                   justify="center",
                   overrelief="raised",
                   padx=10,
                   pady=5,
                   width=15,
                   wraplength=100)

button.pack(padx=20, pady=20)

root.mainloop()
