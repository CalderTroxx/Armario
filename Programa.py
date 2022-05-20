from tkinter import *

ventana = Tk()
ventana.title("Hola Mundo")
ventana.geometry("600x600")

label = Label (ventana, text="Maico solo juega")
label2 = Label (ventana, text="Maico deja de jugar")
label.pack()

boton = Button (ventana, text="Apretame")
boton.pack()
if(boton):
    label2.pack()




ventana.mainloop()


