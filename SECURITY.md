# SECURITY.md

# Security Policy for SzymOS Kernel

The security of SzymOS Kernel and its users is important to us. This document outlines how to report security vulnerabilities and the project’s security practices.

---

## Supported Versions

| Version | Supported |
| ------- | --------- |
| 0.2.x   | Yes       |
| 0.1.x   | No        |

Only the latest minor release in the 0.2.x series is supported for security updates.

---

## Reporting a Vulnerability

If you discover a security vulnerability in SzymOS Kernel, please follow these steps:

1. **Do not create a public issue** on GitHub for security vulnerabilities.
2. Contact the maintainers privately:

   * Email: `szymdows-programming@outlook.com` (replace with a real contact if available)
   * Subject: `SzymOS Kernel Security Report`
3. Include the following information in your report:

   * Detailed description of the vulnerability
   * Steps to reproduce
   * Potential impact
   * Suggested fixes (optional)
4. We will acknowledge your report within 48 hours and provide guidance on patching or mitigation.

---

## Security Practices

* **Code Review:** All contributions are reviewed to prevent security issues.
* **No Hardcoded Secrets:** The kernel and its modules do not contain passwords, keys, or sensitive data.
* **Minimal Privilege:** Only essential hardware access is implemented in drivers.
* **QEMU Testing:** Testing in QEMU is encouraged before deploying to real hardware.

---

## Disclosure Policy

* We follow a responsible disclosure policy.
* Vulnerabilities are fixed promptly, and fixes are released in the next minor version.
* Public acknowledgment of contributors reporting security issues is optional.

---

Thank you for helping keep SzymOS Kernel secure!
