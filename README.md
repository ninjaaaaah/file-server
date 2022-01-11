# File Server

A file server is a server responsible for managing the read/write access of the files in a network.

This file server handles three different user requests and utilizes threads and semaphores to achieve concurrency and persistence.

## Dependencies

This file server is written in C so gcc should be avaiable to be used for compilation.

The file server was also tested in WSL Ubuntu-20.04 but it should work on any distros of Linux and Windows as long as gcc is available.

## Compilation and Execution

The _file_server.c_ file is compiled using the gcc command.

```
gcc file_server.c -lpthread -o file_server
```

And is executed by running the command below.

```
./file_server
```

## Usage

Testing could be done in two ways. 

- First is by directly inputing values upon execution of the file_server binary file

- Second is by piping an input file onto the file_server binary file.

    It is done by following the command below.

    ```
    cat test.in | ./file_server
    ```

## Supported commands

The file server would be able to support the following commands:

### write _\<path/to/file>_ _\<string>_

- The write command accepts two parameters, `<path/to/file>` and `<string>`.

- Both paramaters are at most 50 characters in length and contain alphanumeric characters (excluding spaces for the first parameter).

- When the `<path/to/file>` exists, this command appends the `<string>` at the end of the contents of the file.

- Otherwise, this command creates the file, and writes the string input to the created file.

### read _\<path/to/file>_

- The read command accepts a single argument `<path/to/file>`.

- The parameter is at most 50 characters in length and contains alphanumeric characters (excluding spaces).

- When the file exists, it appends the following to a file named **"read.txt"**:

    ```
    <entire_command>:   <file_contents><newline>
    ```

- Otherwise, it appends: 

    ```
    <entire_command>:   FILE DNE<newline>
    ```

### empty _\<path/to/file>_

- The empty command accepts a single argument `<path/to/file>`.

- The parameter is at most 50 characters in length and contains alphanumeric characters (excluding spaces).

- When the file exists, it appends the following to a file named **"read.txt"**:

    ```
    <entire_command>:   <file_contents><newline>
    ```

    and empties the contents of the file

- Otherwise, it appends: 

    ```
    <entire_command>:   FILE DNE<newline>
    ```

## Attributions

This project was made in accomplishment of the requirements of our CS 140 class in the University of the Philippines Diliman.

## License
[MIT](https://choosealicense.com/licenses/mit/)