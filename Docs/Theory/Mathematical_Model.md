# Mathematical Model & Control Design

**Project:** Aether-Lock  
**Type:** Active Magnetic Levitation System (SISO)  
**Method:** Model-Based Design

---

## 1. Physical System Modeling

The system consists of a ferromagnetic object (the "Angel") suspended by an electromagnet. The objective is to control the vertical position of the object by modulating the current flowing through the coil.

### 1.1 Coordinates and Forces
We define the vertical axis $x$ pointing **downwards**, with the origin $x=0$ located at the bottom face of the electromagnet.

*   $x(t)$: Vertical position of the object [m]
*   $i(t)$: Current in the electromagnet coil [A]

The forces acting on the object are:
1.  **Gravity ($F_g$):** Acts downwards (positive $x$ direction).
    $$ F_g = m \cdot g $$
2.  **Magnetic Force ($F_m$):** Acts upwards (negative $x$ direction). Since the object is a permanent magnet, the force is proportional to the current (not squared) and inversely proportional to the distance raised to a power $n$ (typically 3 or 4 for dipole interactions).
    $$ F_m(x, i) = K_{mag} \frac{i(t)}{x(t)^n} $$

### 1.2 Non-Linear Differential Equation
Applying Newton's Second Law ($F = ma$):

$$ m \ddot{x}(t) = m g - K_{mag} \frac{i(t)}{x(t)^n} $$

This is a **Non-Linear Differential Equation**.

### 1.3 System Parameters
The numerical values for the physical constants ($m$, $g$, $C$, etc.) are strictly defined in the project configuration.

> **Reference:** For the current values used in the simulation and firmware, please refer to the auto-generated table:  
> [**📄 System Parameters Table**](System_Parameters.md)

---