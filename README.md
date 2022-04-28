# Operating-Systems-Ex3
Create programms that run concurrently 

There is a chef that coordinates the production of salads and uses saladmakers.

Saladmakers wait for ingredients to be delivered in order to make salads.

There is one chef, 3 saladmakers. They have to make a specific number of salads, each one containing 3 specific ingredients. The saladmakers take
their ingridients from the same table. In order for everything to work correctly we use POSIX semaphores and shared memory.

In the end we have to find the number of total salads, the number of salads of each saladmaker, time logs for each saladmaker, a list of time intervals 
when at least 2 processes worked in the same time.
