# BASIC++ Virtual Machine & Mock BIOS Architecture

BASIC++ provides a powerful abstraction layer allowing multiple simulated 16-bit or 32-bit hardware environments to run concurrently within a single host instance. This replaces the legacy DIALECT subsystem, enabling accurate emulation of platforms such as the IBM PC, PCjr, XT, and AT without cluttering the global interpreter space.

## The Virtual Machine Architecture

Each Virtual Machine (VM) in BASIC++ operates in complete isolation, running its own instance of the VirtualMachine structure. This encapsulates:
*   **BasicTask**: The core multitasking unit allowing the VM to operate preemptively or cooperatively in the background.
*   **SegmentedMemContext**: An isolated 16MB virtual memory space divided into standard memory banks (RAMBANKs), preventing memory collision between environments.
*   **MockBiosContext**: A self-contained implementation of essential PC BIOS and DOS INT 21H interrupts, simulating low-level system services.
*   **VDevRegistry**: A dedicated virtual device namespace preventing alias conflicts (e.g., COM1 in VM1 is distinct from COM1 in VM2).

Because VMs render to off-screen, headless buffers by default, they can execute completely unnoticed until the user connects a console view to them.

---

## Utilizing the VM Command

The VM statement provides lifecycle management over these environments.

### 1. VM CREATE
Instantiates a new virtual machine in memory but does not begin execution.

**Syntax:**
VM CREATE "name", "architecture"

**Example:**
VM CREATE "DOS1", "IBM PC"

### 2. VM CONFIG
Adjusts the hardware layout, BIOS parameters, or peripheral mappings prior to boot.

**Syntax:**
VM CONFIG "name", parameter, value

**Example:**
VM CONFIG "DOS1", MEM_SIZE, 640

### 3. VM START
Begins execution of the VM. By default, it runs headlessly as a background task.

**Syntax:**
VM START "name"

### 4. VM ATTACH
Maps the host's primary graphical display and keyboard input directly into the running VM's VDevRegistry, bringing it to the foreground.

**Syntax:**
VM ATTACH "name"

### 5. VM REBOOT / VM STOP
Cycles the machine's state or terminates the instance entirely.

**Syntax:**
VM REBOOT "name"
VM STOP "name"

---

## Expanding the Power of the VM

The VM architecture in BASIC++ is designed for futureproofing. We are continuously integrating advanced capabilities that will expand its reach far beyond standard emulation:

1.  **State Management (VM SNAPSHOT / VM RESTORE):**
    Future iterations will allow serializing the entire VirtualMachine memory, task stack, and registers to disk. This means an emulated program can be paused in BASIC++, saved to a .vms file, and completely restored later without losing a single cycle of progress.
2.  **Cycle Throttling (VM SPEED):**
    Providing a retro-accurate experience requires accurate CPU timings. The planned VM SPEED <pct> capability will allow developers to manually choke or boost the execution loop of a specific background VM to simulate older processors (e.g., 4.77 MHz 8088).
3.  **Hardware Interrupt Injection (VM INTERRUPT):**
    Developers will be able to inject simulated hardware interrupts manually via VM INTERRUPT <num>, facilitating the testing of interrupt handlers inside the VM without relying on external devices triggering them.
4.  **Resource Sharing (VM SHARE & Filesystems):**
    Using VM SHARE <bank>, multiple isolated VMs will be able to map a single RAMBANK into their respective memory contexts for ultra-fast inter-VM communication. Furthermore, isolated VMs will support mounting shared host directories via the VFS.
