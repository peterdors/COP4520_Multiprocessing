How to Run 
================================================================================
You can compile and run any version of the program by changing directory into
their respective version folder and calling: 

javac DiningPhilosophers.java && java DiningPhilosophers


Summary of Approach
================================================================================
GENERAL APPROACH: We use the Java Object class for creating our chopsticks and 
create our own Philosopher class that implements the Runnable interface in Java
for threading. We create the chopstick and philosopher objects in the 
DiningPhilosophers main function. Assigning each philosopher their respective 
left and right chopsticks (version 3 and 4 use first and second notation). We 
then begin to run each philosopher in their own thread. 

The Philosopher class will utilize the 'synchronized' keyword built into Java, 
using mutual exclusion and locking the chopstick object until it becomes 
available for the next philosopher to use.  

Correctness and Efficiency of Algorithm
================================================================================
VERSION 1: We show this has a case for deadlocking as we circularly assign each 
philosopher a left and right chopstick. What causes the deadlock is when each 
philosopher picks up their left chopstick, and are all waiting for their right 
chopstick to be released so they can begin eating. Mainly it occurs once the 
last philosopher has been assigned the same right chopstick that the first
philosopher has been assigned for their left chopstick. 

VERSION 2: We show this program solves the deadlocking issue by assigning the 
last philosopher to pickup their right chopstick instead of their left 
chopstick. While all others are still to pickup their left chopstick first. This
shows to break the circularity of the problem that causes the deadlocking. 

VERSION 3: We show this program to solve the issue of starvation by assigning 
even numbered philosophers to pickup their right chopstick first and having the
odd numbered philosophers to pickup their left chopstick first. This allows 
every philosopher to have a chance to eat.  

VERSION 4: We show this program to solve the issue of starvation by similarly 
assigning even numbered philosophers to pickup their right chopstick first and 
the odd numbered philosophers to pickup their left chopstick first. We will add
this capability by allowing input for the program to have any number of 
philosophers. 


Summary of Experimental Evaluation
================================================================================
VERSION 1: We show this to occur as the program does not output any progress 
from the philosophers when they all become hungry. 

VERSION 2: We show the deadlock-free approach by having output continue once 
the philosophers become hungry. 

VERSION 3: We show the starvation-free approach by allowing every philosopher a
chance to eat. 

VERSION 4: We show the starvation-free approach by allowing every philosopher a
chance to eat. 
