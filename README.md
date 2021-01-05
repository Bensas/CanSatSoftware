# CanSat Software
This repo contains software developed by Team L.A.T.A. for the CanSat 2021 competition.
It contains the following pieces of software:
# GroundModule
ElectronJS Application to be run on a laptop to communicate with the Container, issuing commands, receiving, saving and relaying telemetry data, as well as read mock data and send it to the Container for simulation purposes.
## Compilation
Requirements:
- NodeJS
- NPM

To run the Application, enter the repo and run the following commands:
1) ```cd GroundModule```
2) ```npm install```
3) ```npm start```

# FlightSoftware
Arduino software for a the Container and the two Payloads, along with common libraries they both use.
