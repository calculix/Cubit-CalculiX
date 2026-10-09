# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#

PRESSURE = "p"
VELOCITY = "U"
PRESSURE_ADJ = "pa"
VELOCITY_ADJ = "Ua"

DIV_VELOCITY = "div(phi,U)"
DIV_VELOCITY_ADJOINT = "div(-phi,Ua)"

FOAM_FIRST_ORDER_DIV_SCHEME = "bounded Gauss upwind"
FOAM_SECOND_ORDER_DIV_SCHEME = "bounded Gauss linearUpwind"