# Mathematical Model & Control Design

**Project:** Aether-Lock  
**Type:** Active Magnetic Levitation System (SISO)  
**Method:** Model-Based Design

---

## 1. Physical System Modeling

The system consists of a ferromagnetic object (the "Angel") suspended by an electromagnet. The objective is to control the vertical axis $x$ (pointing downwards, origin at the coil face).

Applying Newton's Second Law ($F = ma$):

$$ m \ddot{x}(t) = F_{gravity} - F_{magnetic}(x, i) $$

$$ m \ddot{x}(t) = m g - F_m(x, i) $$

To design a controller, we first need an accurate model of the magnetic force $F_m$.

---

## 2. Magnetic Force Modeling

The magnetic force acting on a permanent magnet (dipole) aligned with the field is given by the gradient of the magnetic field:

$$ \mathbf{F} = \nabla (\mathbf{m} \cdot \mathbf{B}) \implies F_x = m_{mag} \cdot \frac{dB_x}{dx} $$

Where $m_{mag}$ is the Magnetic Dipole Moment of the object.

### 2.1 Magnetic Dipole Moment Identification

To resolve the force equation $\mathbf{F} = \nabla (\mathbf{m} \cdot \mathbf{B})$, the magnitude of the Angel's magnetic dipole moment ($m_{mag}$) must be determined. 

Instead of experimental estimation, we derive this analytically using the magnet's volume and the material's remanence ($B_r$).

**Data Source:**
For standard Sintered Neodymium-Iron-Boron (NdFeB) magnets of **Grade N35**, the physical data is referenced from *Supermagnete* [2].
*   **Remanence ($B_r$):** $1.2 \, T$ (Tesla)
*   **Magnet Volume ($V$):** $1.582874 \cdot 10^{-7} \, m^3$ (Measured)
*   **Vacuum Permeability ($\mu_0$):** $4\pi \cdot 10^{-7} \, T\cdot m/A$

**Calculation:**
$$ m_{mag} = \frac{B_r \cdot V}{\mu_0} $$

Substituting the numerical values:

$$ m_{mag} = \frac{1.2 \cdot (1.582874 \cdot 10^{-7})}{4\pi \cdot 10^{-7}} \approx \mathbf{0.1511} \, A \cdot m^2 $$

This value is used as a constant in the simulation model.

### 2.1 Ground Truth (Thick Solenoid Model)
Since the electromagnet has a significant thickness (inner radius $R_1 \neq$ outer radius $R_2$), we use the **Finite Thick Solenoid** model derived from the Biot-Savart law.

The axial magnetic field $B_x$ at distance $x$ is (Ref. [1], Eq. 12):

$$ B_x(x) = \frac{\mu_0 N i}{2L (R_2 - R_1)} \left[ (x + L/2) \ln \left( \frac{R_2 + \sqrt{R_2^2 + (x+L/2)^2}}{R_1 + \sqrt{R_1^2 + (x+L/2)^2}} \right) - (x - L/2) \ln \left( \frac{R_2 + \sqrt{R_2^2 + (x-L/2)^2}}{R_1 + \sqrt{R_1^2 + (x-L/2)^2}} \right) \right] $$

*Note: This complex formula is implemented in [identify_physics.m](../../Simulation/MATLAB/identify_physics.m) to perform data fitting and extract the simplified model parameters.*

$$ F_{magn} = -\frac{I\,N\,m_{\mathrm{mag}}\,\mu _{0}\,\left(\ln\left(\frac{R_{2}+\sqrt{{R_{2}}^2+{\left(\frac{L}{2}+z\right)}^2}}{R_{1}+\sqrt{{R_{1}}^2+{\left(\frac{L}{2}+z\right)}^2}}\right)-\ln\left(\frac{R_{2}+\sqrt{{\left(\frac{L}{2}-z\right)}^2+{R_{2}}^2}}{R_{1}+\sqrt{{\left(\frac{L}{2}-z\right)}^2+{R_{1}}^2}}\right)-\frac{\left(R_{1}+\sqrt{{\left(\frac{L}{2}-z\right)}^2+{R_{1}}^2}\right)\,\left(\frac{L-2\,z}{2\,\sqrt{{\left(\frac{L}{2}-z\right)}^2+{R_{2}}^2}\,\left(R_{1}+\sqrt{{\left(\frac{L}{2}-z\right)}^2+{R_{1}}^2}\right)}-\frac{\left(R_{2}+\sqrt{{\left(\frac{L}{2}-z\right)}^2+{R_{2}}^2}\right)\,\left(L-2\,z\right)}{2\,\sqrt{{\left(\frac{L}{2}-z\right)}^2+{R_{1}}^2}\,{\left(R_{1}+\sqrt{{\left(\frac{L}{2}-z\right)}^2+{R_{1}}^2}\right)}^2}\right)\,\left(\frac{L}{2}-z\right)}{R_{2}+\sqrt{{\left(\frac{L}{2}-z\right)}^2+{R_{2}}^2}}+\frac{\left(R_{1}+\sqrt{{R_{1}}^2+{\left(\frac{L}{2}+z\right)}^2}\right)\,\left(\frac{L}{2}+z\right)\,\left(\frac{L+2\,z}{2\,\left(R_{1}+\sqrt{{R_{1}}^2+{\left(\frac{L}{2}+z\right)}^2}\right)\,\sqrt{{R_{2}}^2+{\left(\frac{L}{2}+z\right)}^2}}-\frac{\left(R_{2}+\sqrt{{R_{2}}^2+{\left(\frac{L}{2}+z\right)}^2}\right)\,\left(L+2\,z\right)}{2\,{\left(R_{1}+\sqrt{{R_{1}}^2+{\left(\frac{L}{2}+z\right)}^2}\right)}^2\,\sqrt{{R_{1}}^2+{\left(\frac{L}{2}+z\right)}^2}}\right)}{R_{2}+\sqrt{{R_{2}}^2+{\left(\frac{L}{2}+z\right)}^2}}\right)}{2\,L\,\left(R_{1}-R_{2}\right)}$$

### 2.2 Simplified Design Model
For the control algorithm, we approximate the force behavior around the operating point using a Power Law model:

$$ F_m(x, i) \approx K_{mag} \frac{i(t)}{x(t)^n} $$

The parameters $K_{mag}$ and the exponent $n$ are identified by fitting this curve against the Ground Truth model in MATLAB.

---

## 3. Equilibrium & Linearization

Substituting the simplified model into Newton's law:

$$ m \ddot{x} = m g - K_{mag} \frac{i}{x^n} $$

This is a non-linear differential equation. We must linearize it around a fixed operating point (Equilibrium).

### 3.1 Equilibrium Point
We define the desired target position $\bar{x}$.
At equilibrium, acceleration is zero ($\ddot{x} = 0$) and current is constant ($\bar{i}$).

$$ 0 = m g - K_{mag} \frac{\bar{i}}{\bar{x}^n} $$

Solving for the equilibrium current $\bar{i}$:

$$ \bar{i} = \frac{m g \bar{x}^n}{K_{mag}} $$

### 3.2 Linearization (Taylor Expansion)
We define small perturbations around the equilibrium:
*   $x(t) = \bar{x} + \tilde{x}(t)$
*   $i(t) = \bar{i} + \tilde{i}(t)$

Expanding the non-linear term using Taylor Series (first order):

$$ F_m(x, i) \approx F_m(\bar{x}, \bar{i}) + \left( \frac{\partial F_m}{\partial x} \right)_{eq} \tilde{x} + \left( \frac{\partial F_m}{\partial i} \right)_{eq} \tilde{i} $$

Calculating the partial derivatives:
1.  **Current Gain:** $\frac{\partial}{\partial i} \left( K \frac{i}{x^n} \right) = \frac{K}{\bar{x}^n} = \frac{mg}{\bar{i}}$
2.  **Position Stiffness:** $\frac{\partial}{\partial x} \left( K \frac{i}{x^n} \right) = -n K \frac{\bar{i}}{\bar{x}^{n+1}} = -n \frac{mg}{\bar{x}}$

Substituting back into the differential equation (noting the negative sign of $F_m$):

$$ m \ddot{\tilde{x}} = \left( n \frac{mg}{\bar{x}} \right) \tilde{x} - \left( \frac{mg}{\bar{i}} \right) \tilde{i} $$

Dividing by $m$:

$$ \ddot{\tilde{x}} = \left( \frac{n g}{\bar{x}} \right) \tilde{x} - \left( \frac{g}{\bar{i}} \right) \tilde{i} $$

### 3.3 Transfer Function
Applying Laplace Transform ($s^2 X(s)$ for acceleration):

$$ s^2 X(s) - \frac{ng}{\bar{x}} X(s) = - \frac{g}{\bar{i}} I(s) $$

The Open-Loop Transfer Function $G(s) = \frac{X(s)}{I(s)}$ is:

$$ G(s) = \frac{- \frac{g}{\bar{i}}}{s^2 - \frac{ng}{\bar{x}}} $$

**Stability Analysis:**
The poles are at $s = \pm \sqrt{\frac{ng}{\bar{x}}}$.
Since one pole is real and positive, the system is **Open-Loop Unstable**.

---

## 4. System Parameters Reference
The numerical values used for simulation and code generation are strictly defined in the configuration file.

> [**📄 View System Parameters Table**](System_Parameters.md)

---

## References
1.  Fuso, F. (2015). *Campo magnetico prodotto da un solenoide*. Dipartimento di Fisica, Università di Pisa. [Online PDF](https://osiris.df.unipi.it/~fuso/dida/solenoide.pdf)
2.  Supermagnete. *Physical magnet data*. Retrieved December 2025 from [supermagnete.de/eng/physical-magnet-data](https://www.supermagnete.de/eng/physical-magnet-data)
