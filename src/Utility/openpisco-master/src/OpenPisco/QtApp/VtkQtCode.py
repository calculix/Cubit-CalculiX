# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
import vtk


class MouseInteractorHighLightActor(vtk.vtkInteractorStyleTrackballCamera):
    """Main class to treat mouse interactions."""
    def __init__(self,parent=None, app=None):
        self.parent = parent
        self.mod = "none"
        self.LastPickedActor = None
        self.LastPickedProperty = vtk.vtkProperty()
        if parent is not None:
            self.parent.AddObserver("KeyPressEvent",self.keyPressEvent)
        self.AddObserver("LeftButtonPressEvent",self.leftButtonPressEvent)
        self.app = app

    def keyPressEvent(self,obj,event):
        key = obj.GetKeyCode() #Does not work
        #print("key %s" % key)
        if key == "+" :
            self.mod = "add"
            self.app.statusBar().showMessage('Edit: Addition active')
        if key == "e" :
            self.mod = "explore"
            self.app.statusBar().showMessage('Edit: Explore click to know pos')
        elif key == "-" :
            self.mod = "minus"
            self.app.statusBar().showMessage('Edit: Subtraction active')
        elif key == "0":
            self.mod = "none"
            self.app.statusBar().showMessage('Edit: Edition inactive')
        elif key == "*":
            self.mod = "smooth"
            self.app.statusBar().showMessage('Edit: Smooth active')

        elif key == "a":
            self.app.colormapBy = self.app.colormapBy+1
            self.app.vtk.update3D.emit()
        elif key == "A":
            self.app.colormapBy = max(self.app.colormapBy-1,0)
            self.app.vtk.update3D.emit()


    def leftButtonPressEvent(self,obj,event):
        clickPos = self.GetInteractor().GetEventPosition()

        if self.mod != "none":
          picker = vtk.vtkPropPicker()
          picker.Pick(clickPos[0], clickPos[1], 0, self.GetDefaultRenderer())

          # get the new
          self.NewPickedActor = picker.GetActor()

          # If something was selected
          if self.NewPickedActor:
            self.app.AddSphere(picker.GetPickPosition(),self.mod)

            # If we picked something before, reset its property

            if self.LastPickedActor:
                self.LastPickedActor.GetProperty().DeepCopy(self.LastPickedProperty)

            # Save the property of the picked actor so that we can
            # restore it next time
            self.LastPickedProperty.DeepCopy(self.NewPickedActor.GetProperty())
            # Highlight the picked actor by changing its properties
            self.NewPickedActor.GetProperty().SetColor(0.0, 1.0, 0.0)
            self.NewPickedActor.GetProperty().SetDiffuse(1.0)
            self.NewPickedActor.GetProperty().SetSpecular(0.0)
            # save the last picked actor
            self.LastPickedActor = self.NewPickedActor

        self.OnLeftButtonDown()

def CheckIntegrity():
    MouseInteractorHighLightActor()
    return "ok"

if __name__ == '__main__':
    print(CheckIntegrity())
