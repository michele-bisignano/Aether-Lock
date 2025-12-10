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

To resolve the force equation, the magnitude of the Angel's magnetic dipole moment ($m_{mag}$) must be determined. Instead of experimental estimation, we derive this analytically using the magnet's volume and the material's remanence ($B_r$).

**Data Source:**
For standard Sintered Neodymium-Iron-Boron (NdFeB) magnets of **Grade N35**, the physical data is referenced from *Supermagnete* [2].
*   **Remanence ($B_r$):** $1.2 \, T$ (Tesla)
*   **Magnet Volume ($V$):** $1.58 \cdot 10^{-7} \, m^3$ (Measured)
*   **Vacuum Permeability ($\mu_0$):** $4\pi \cdot 10^{-7} \, T\cdot m/A$

**Calculation:**
$$ m_{mag} = \frac{B_r \cdot V}{\mu_0} \approx \mathbf{0.1511} \, A \cdot m^2 $$

This value is used as a constant in the simulation model.

### 2.2 Ground Truth (Thick Solenoid Model)
Since the electromagnet has a significant thickness (inner radius $R_1 \neq$ outer radius $R_2$), we use the **Finite Thick Solenoid** model derived from the Biot-Savart law.

The axial magnetic field $B_x$ at distance $z$ (or $x$) is (Ref. [1], Eq. 12):

$$ B_x(z) = \frac{\mu_0 N i}{2L (R_2 - R_1)} \left[ (z + L/2) \ln \left( \frac{R_2 + \sqrt{R_2^2 + (z+L/2)^2}}{R_1 + \sqrt{R_1^2 + (z+L/2)^2}} \right) - (z - L/2) \ln \left( \frac{R_2 + \sqrt{R_2^2 + (z-L/2)^2}}{R_1 + \sqrt{R_1^2 + (z-L/2)^2}} \right) \right] $$

### 2.3 Core Amplification Factor ($\mu_{eff}$)
The formula above calculates the field in a vacuum (air core). However, the P25/20 electromagnet has a ferromagnetic core which significantly amplifies the magnetic flux density. To match the real-world performance, we introduce an **Effective Permeability Factor** ($\mu_{eff}$).

**Estimation via Reverse Engineering:**
According to the datasheet, the holding force at contact is $F_{hold} \approx 80\,N$. Calculating the theoretical field in air ($B_{air}$) and comparing it with the field required to generate 80N ($B_{real} \approx 1.3 T$), we estimate an amplification factor:

$$ \mu_{eff} \approx 50 $$

In the MATLAB model, the theoretical field is multiplied by this factor to obtain the true force.

### 2.4 Exact Force Derivation (The Gradient)
Combining the gradient of the amplified field with the dipole moment, the explicit force equation used for parameter identification is:

$$ F_{magn} = -\mu_{eff} \cdot \frac{I\,N\,m_{\mathrm{mag}}\,\mu _{0}}{2\,L\,\left(R_{1}-R_{2}\right)} \left[\ln\left(\frac{R_{2}+\sqrt{{R_{2}}^2+{\left(\frac{L}{2}+z\right)}^2}}{R_{1}+\sqrt{{R_{1}}^2+{\left(\frac{L}{2}+z\right)}^2}}\right)-\ln\left(\frac{R_{2}+\sqrt{{\left(\frac{L}{2}-z\right)}^2+{R_{2}}^2}}{R_{1}+\sqrt{{\left(\frac{L}{2}-z\right)}^2+{R_{1}}^2}}\right) + \dots \right] $$

*(Note: The full derivative expansion is handled symbolically in [identify_physics.m](../../Simulation/MATLAB/identify_physics.m)).*

### 2.5 Simplified Design Model
For the control algorithm design and real-time execution, we approximate this complex behavior around the operating point using a Power Law model:

$$ F_m(x, i) \approx K_{mag} \frac{i(t)}{x(t)^n} $$

**Justification:**
1.  **Linearity with Current ($i$):** Valid for permanent magnets (Dipole interaction).
2.  **Effective Exponent ($n$):** By fitting this model to the ground truth data around the equilibrium point, the parameter $n$ captures the **effective decay rate**, implicitly compensating for the coil's geometry.

---

## 3. Equilibrium & Linearization

Substituting the simplified model into Newton's law:

$$ m \ddot{x} = m g - K_{mag} \frac{i}{x^n} $$

### 3.1 Equilibrium Point
At equilibrium ($\ddot{x} = 0$, $x = \bar{x}$), the required current $\bar{i}$ is:

$$ \bar{i} = \frac{m g \bar{x}^n}{K_{mag}} $$

### 3.2 Linearization (Taylor Expansion)
Expanding around $(\bar{x}, \bar{i})$:

$$ m \ddot{\tilde{x}} = \left( n \frac{mg}{\bar{x}} \right) \tilde{x} - \left( \frac{mg}{\bar{i}} \right) \tilde{i} $$

Dividing by $m$:

$$ \ddot{\tilde{x}} = \left( \frac{n g}{\bar{x}} \right) \tilde{x} - \left( \frac{g}{\bar{i}} \right) \tilde{i} $$

### 3.3 Transfer Function
Applying Laplace Transform:

$$ s^2 X(s) - \frac{ng}{\bar{x}} X(s) = - \frac{g}{\bar{i}} I(s) $$

The Open-Loop Transfer Function $G(s) = \frac{X(s)}{I(s)}$ is:

$$ G(s) = \frac{- \frac{g}{\bar{i}}}{s^2 - \frac{ng}{\bar{x}}} $$

**Stability Analysis:**
The poles are at $s = \pm \sqrt{\frac{ng}{\bar{x}}}$. Since one pole is real and positive, the system is **Open-Loop Unstable**.

---

## References
1.  Fuso, F. (2015). *Campo magnetico prodotto da un solenoide*. Dipartimento di Fisica, Università di Pisa. [Online PDF](https://osiris.df.unipi.it/~fuso/dida/solenoide.pdf)
2.  Supermagnete. *Physical magnet data*. Retrieved December 2025 from [supermagnete.de](https://www.supermagnete.de/eng/physical-magnet-data)