# Formula 1 Racecar Engine Simulator

An application that simulates the interactions and reactions a RTOS must perform to reliably control the engine of a Formula 1 racing car. 

## Authors

- [Iain Found](https://github.com/ifoundcarleton)
- [George Mojeed](https://github.com/mojeed123)
- [Danny Nemec](https://github.com/daniel-nemec-carleton)
- [Fareha Sultan](https://www.github.com/farehasultan)

## Documentation

### Setting up and running the backend

Note: QNX Software Development Platform 7.1 Education was used for the backend.

- Launch the QNX Momentics IDE.
- Under File, import the backend folder as a project.
- Run your virtual machine and start SSH session.
- Right-click on the project folder to build it . Move the executable file inside the tmp folder of the virtual machine using the Target File System Navigator.
- Move into the tmp directory using the terminal and type './backend' to run the backend.

### Finding Your Connection IP

- Inside the Momentics IDE Workplace, under the Target Navigator tab, right-click on the virtual machine (ex: vm1) and click on properties. Make note of the IP address under "Hostname or IP" as you will need this to setup the middleman.js.

### Setting up and running the Middleman

Note: Node v16.15.1 was used for this.

- Open the Middleman folder inside your preferred IDE.
- First and foremost, edit line 30 with your corresponding IP Address. Save your changes.
- Open a terminal and move into the middleman folder. From there type in 'npm i' to install all the required packages.
- Next, run the middleman by typing in 'node .\middleman.js'
- Once that is running, open browser of your choice and enter the following url http://localhost:3000/engine.
- Enter a throttle setting and click send to receive RPM data of the engine from the backend.
- Repeat the step above with a different throttle setting and see how the RPM changes!
