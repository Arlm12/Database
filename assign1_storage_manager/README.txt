CS525 Advanced Database Organization Fall 2024
Assignment 1 - Storage Manager

Information about the team:
Group 22: Arunachalam Barathidasan(abarathidasan@hawk.iit.edu)
          Sejal Srivastav (ssrivastav@hawk.iit.edu)
          Prathiksha Shirsat (pshirsat@hawk.iit.edu)

Contribution: The members of the team have equal contibution. 
Outcome: This assignment helped us in understanding how concepts of filehandling, file manipulation and error handling. 
We also learnt two data structure concepts that are available in the assignment. The make file is also created which is
used to compile and run the entire assignement. In the upcoming parts you would see how to run the program.

Running the program: 
If you are using VScode here are the steps below:
1. Open the assignment folder
2. open up terminal by pressing ctrl + ` 
3. In the terminal enter the command make
4. Inorder to delete the files enter the command make clean

If you are using CMD to run the program:
1. In the command prompt enter cd and move into the assignment directory
2. Make sure that the present working directory is assign1_storage_manager
3. Now enter the command make
4. Now to remove/delete the files enter command make clean

Explanation about the Makefile:
The make file has the following
1. The warning flag used is -Wall. This only gives the common warnings
2. The CC=GCC represents that we need to use C compiler.
3. Now the files totalfiles comprises of all the files in the assingment that is dberror.c storage_mgr.c and test_assign1_1.c
4. This excludes the header files that are present with the .h extension
5. These files are now put into tfiles which has the .o extension
6. we start by building the assignment 
7. We also print multiple statements throughout to know which step is being executed
8. Now the files are created in the next step.
9. using the del command we remove the files when make clean command is run

Note: We did this assignment in Windows OS so inorder for the make clean command to run we use del. 
If it is mac then the command rm -f that is remove force would be used.

Explanation of the code: 
1. In the code we have retained the variable names, function names as given in the header file. 
2. The code starts with importing the required libraries that are going to be used. 
3. We use the init function is used to intialize the code. 
4. Function createpagefile creates a empty file which is essential.
5. Now the functions starts filling the page with empty blocks.
6. Here we through a number of errors like file not found and write failed incase of exceptions.
7. There are other functions that are used to open the page file, close the page of the file, delete the page etc.
8. And afterwards we start to read the file by reading the block like reading the present block, next block, previous block,
reading the first block, last block etc.
9. And finally we have a set of functions that wirtes block, enters an empty block and makes sure that the entire size does not exceed. 

