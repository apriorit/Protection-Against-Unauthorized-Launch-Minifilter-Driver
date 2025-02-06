# Kernel Driver Minifilter: Protection Against Unauthorized Process Launch or File Open

This repository contains a simple Windows minifilter kernel driver that intercepts file create (IRP_MJ_CREATE) operations to block the launch of a specific executable or file. 
The solution was made in the context of Apriorit blog article ["How to Develop a Windows File System Minifilter Driver: Complete Tutorial"](https://www.apriorit.com/dev-blog/675-driver-windows-minifilter-driver-development-tutorial)

The project illustrates how to:
- Register a minifilter driver.
- Intercept file operations.
- Make a selective filtering of calls.
- Deny access to prevent process creation or file opening.

---

## Requirements

- Visual Studio 2019 or higher 
- [Windows Driver Kit (WDK)](https://learn.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk)
- A test VM with Windows in **Test Mode** (or with code signing properly set up)  

---

## Getting Started

1. **Clone the Repository** 
2. **Open project in Visual Studio** 
3. **Build the solution**  
4. **Sign the Driver (Optional/Test Mode)**  

---
## Installation

1. **Copy the Driver binaries**  
   Copy binaries to your test VM: `fsminifilter.cat`, `FsMinifilter.sys`, `FsMinifilter.inf`

2. **Install the Service**  
   Right-click on `FsMinifilter.inf` and choose "Install" option

3. **Start the Service from CMD**  
   ```cmd
   sc start FsMinifilter
   ```

---

## Uninstallation

1. **Stop the Service from CMD**  
   ```cmd
   sc stop FsMinifilter
   ```

2. **Uninstall the Service from CMD**  
   ```cmd
   pnputil /delete-driver "FsMinifilter.inf" /uninstall /force
   ```

3. **Manually delete your copied files**  
   Delete binaries from your test VM: `fsminifilter.cat`, `FsMinifilter.sys`, `FsMinifilter.inf`

---

## Usage

1. **Edit the Target Process and Target File**  
   - In the `FsMinifilter.cpp`, update the variables `unauthorizedFileName` and `unauthorizedProcessName` of the file and process you want to block (e.g., `passwords.exe` and `msedge.exe`).  
2. **Rebuild the solution**  
3. **Install the driver on test VM**  
4. **Attempt to launch the specified executable or file**

---
 
