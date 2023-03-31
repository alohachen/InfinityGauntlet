# InfinityGauntlet

https://user-images.githubusercontent.com/126397459/228130499-61dc367e-9943-49e1-a97a-840ad6c6f14d.mp4

NOTE: The demo video only demonstrates a rough attack flow, please refer to the paper for details.

We proposes a novel fingerprint brute-force attack on off-the-shelf smartphones in [USENIX Security '23](https://www.usenix.org/conference/usenixsecurity23). We name this new threat model InfinityGauntlet, where Infinity and Gauntlet represent its two core techniques: attempt limit bypassing and fingerprint image hijacking. Adversaries can pass fingerprint authentication with zero knowledge of the victim to unlock the smartphone, log into privacy apps and make payments. We have submitted related vulnerabilities(CAMF and MAL) to seven manufacturers, and all have been confirmed and fixed. In order to fill the gap in the research tools of smartphone fingerprint authentication, we decided to open source the tool in this repository to the academic community. Hope InfinityGauntlet can inspire the industry to improve the security of biometric authentication. 

Some of our other research on the security of biometric authentication:

1. https://www.blackhat.com/us-19/briefings/schedule/#biometric-authentication-under-threat-liveness-detection-hacking-16130
2. Demonstration of presentation attacks on multiple fingerprint devices. https://2019.geekpwn.org
3. https://threatpost.com/lock-screen-bypass-bug-quietly-patched-in-handsets/139141/
