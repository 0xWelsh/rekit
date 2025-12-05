# REMnux VM Setup for Malware Analysis

## Overview
REMnux is a lightweight Linux distribution (Ubuntu-based) designed specifically for reverse engineering and malware analysis. It provides a safe, isolated environment to analyze malicious software without risking your host system.

## System Requirements
- RAM: 4GB minimum (8GB recommended)
- Disk: 60-80GB
- CPU: 2 cores minimum
- VirtualBox installed

## Setup Procedure

### 1. Check if VirtualBox is Installed
```bash
which virtualbox
```

If not installed:
```bash
sudo apt update
sudo apt install virtualbox
```

### 2. Download REMnux OVA
Download from: https://docs.remnux.org/install-distro/get-virtual-appliance

Or use wget:
```bash
mkdir -p ~/malware-analysis
cd ~/malware-analysis
# Download the latest REMnux OVA (~5GB)
```

### 3. Import REMnux into VirtualBox
```bash
vboxmanage import ~/Downloads/remnux-v7-focal-virtualbox.ova --vsys 0 --vmname "REMnux-Analysis"
```

### 4. Configure Network Isolation (Critical for Safety)
Set network to internal (isolated from internet):
```bash
vboxmanage modifyvm "REMnux-Analysis" --nic1 intnet
```

Other network options:
- Completely disable: `vboxmanage modifyvm "REMnux-Analysis" --nic1 none`
- Enable NAT (for updates only): `vboxmanage modifyvm "REMnux-Analysis" --nic1 nat`

### 5. Start the VM
```bash
vboxmanage startvm "REMnux-Analysis"
```

Or use VirtualBox GUI.

## Default Credentials
- Username: `remnux`
- Password: `malware`

## Safety Best Practices

### Before Analyzing Malware:
1. **Take a snapshot** - Allows you to restore to clean state
   ```bash
   vboxmanage snapshot "REMnux-Analysis" take "Clean-State"
   ```

2. **Verify network isolation** - Ensure VM cannot access internet
   ```bash
   vboxmanage showvminfo "REMnux-Analysis" | grep NIC
   ```

3. **Never share folders** - Disable shared folders between host and VM during analysis

4. **Use snapshots frequently** - Before each analysis session

### Restoring from Snapshot:
```bash
vboxmanage snapshot "REMnux-Analysis" restore "Clean-State"
```

## VM Configuration
- OS: Ubuntu 20.04 LTS (Focal)
- CPUs: 2
- RAM: 4GB
- Disk: 60GB
- Network: Internal (isolated)

## Additional Tips
- REMnux comes pre-installed with malware analysis tools (IDA, Ghidra, strings, etc.)
- Always analyze malware in isolated environment
- For sophisticated malware, consider air-gapped system
- Document your analysis process
- Keep REMnux updated when not analyzing samples

## Updating REMnux
Only update when network is enabled and no malware is present:
```bash
# Enable network temporarily
vboxmanage modifyvm "REMnux-Analysis" --nic1 nat

# Start VM and run inside:
remnux upgrade

# Disable network again
vboxmanage modifyvm "REMnux-Analysis" --nic1 intnet
```

## Resources
- Official Documentation: https://docs.remnux.org
- REMnux Tools: https://docs.remnux.org/discover-the-tools
