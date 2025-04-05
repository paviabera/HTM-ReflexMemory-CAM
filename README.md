# H-AHTM: Hardware-Accelerated Hierarchical Temporal Memory with Reflex Memory

Welcome to the AHTM/H-AHTM repository! This project is the open-source implementation of the methods described in our paper, *Enhancing Biologically Inspired Hierarchical Temporal Memory with Hardware-Accelerated Reflex Memory* (Preprint submitted to Elsevier, April 1, 2025). Our work introduces a novel acceleration framework for Hierarchical Temporal Memory (HTM) systems by integrating a lightweight, biologically inspired Reflex Memory (RM) module along with a hardware-accelerated Content Addressable Memory (CAM) design.

---

## About

Hierarchical Temporal Memory (HTM) mimics the structure and function of the human neocortex for pattern recognition and sequence prediction. However, conventional HTM implementations face computational bottlenecks, especially in the Sequence Memory (SM) module, when dealing with high-frequency data. Our approach improves performance by:

- **Reflex Memory (RM):** Inspired by biological reflex arcs, RM rapidly handles frequent, first-order temporal inferences using a fixed-size dictionary structure.
- **Hardware Acceleration with CAM:** By integrating a CAM module, we achieve sub-centisecond inference speeds, significantly reducing latency.
- **Adaptive Online Learning:** A dedicated control unit balances the use of RM and SM based on real-time performance, ensuring both speed and accuracy.

This combination—termed H-AHTM—makes our framework particularly well-suited for applications such as real-time anomaly detection in finance, IoT data processing, and more.

---

## Features

- **Real-Time Processing:** Rapid anomaly detection and sequence prediction for streaming data.
- **Scalable Architecture:** Efficient handling of large datasets with low memory overhead.
- **Biologically Inspired Design:** Emulates the human brain’s reflex memory to achieve fast, adaptive learning.
- **Hardware Integration:** Leverages CAM technology for improved inference speed and energy efficiency.

---

## Installation

Follow these steps to set up H-AHTM on your local machine:

1. **Clone the Repository:**
   ```bash
   git clone https://github.com/paviabera/hahtm.git
   cd hahtm
2. Run the Docker build in the File

