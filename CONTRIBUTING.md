# Contributing to SzymOS Kernel

Thank you for your interest in contributing to **SzymOS Kernel**! Your contributions help improve the project and make it more robust and educational for everyone.

---

## How to Contribute

### 1. Reporting Issues
- Use the GitHub **Issues** tab to report bugs, hardware compatibility problems, or feature requests.
- Provide as much detail as possible, including steps to reproduce the issue and system setup (QEMU or real hardware).
- Tag issues appropriately (e.g., `bug`, `enhancement`, `documentation`).

### 2. Submitting Pull Requests
- Fork the repository to your account.
- Create a new branch with a descriptive name, e.g., `feature/keyboard-driver` or `fix/vga-output`.
- Make your changes on the branch.
- Run tests on your changes using QEMU or your own setup.
- Ensure your code follows the [Coding Style Guide](docs/coding-style.md).
- Update relevant documentation if your changes introduce new features or alter existing behavior.
- Submit a Pull Request (PR) to the main repository:
  - Include a clear description of what your PR does.
  - Reference any related issues (e.g., `Fixes #23`).

### 3. Documentation and README Updates
- Updates to `README.md` or other documentation are highly appreciated.
- Include explanations of new features, new hardware support, or examples of how to use new modules.
- For major changes, update the **version number** in `include/version.h`.

### 4. Code Standards
- Follow the C and Assembly coding conventions in the [Coding Style Guide](docs/coding-style.md).
- Keep commits small and focused; each commit should address a single issue or feature.
- Comment complex code to help others understand it.
- Avoid committing compiled binaries (`*.o`, `*.bin`, `*.iso`).

### 5. Communication
- Discuss larger features or architecture changes in **GitHub Issues** before implementing.
- Be respectful and constructive in discussions.
- Offer feedback on PRs if requested; collaboration is key.

### 6. Testing Changes
- Use **QEMU** to test kernel builds:
  ```bash
  qemu-system-x86_64 -cdrom SzymOS-kernel-0.2.0.iso
  ```
- Verify that the kernel boots, displays the banner, and initializes the VGA and GDT correctly.
- Check for crashes, warnings, or unexpected behavior.

### 7. Recommended Workflow
1. Fork the repo.
2. Create a branch for your feature/fix.
3. Make changes and test locally.
4. Update documentation if needed.
5. Commit with clear messages.
6. Push your branch and open a Pull Request.
7. Respond to review comments and iterate.

---

Thank you for helping make **SzymOS Kernel** better! Your contributions are valued and appreciated.