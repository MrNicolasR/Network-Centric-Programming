// Nicolas Rubert
// Nar126
// 172007365

// Include files
#include <stdio.h>
#include <stdlib.h>

// Prototyping
int p1(char *filename, char *substring);
int p2(char *filename, char *substring);

// Main function
int main(int argc, char *argv[])
{
    // Variables
    int argCounter;       // Arguements
    char *filename;       // Filename
    int first = 0;        // Flag
    char *flag = argv[1]; // Flag

    // Check if flag is valid
    if (flag[0] == '-')
    {
        if (flag[1] == 'C')
        {
            if (flag[2] == 'H')
            {
                first = -1;
            }
        }
    }

    // If Flag is given
    if (first < 0)
    {
        // File Name
        filename = argv[2];

        // Get the substrings
        for (argCounter = 3; argCounter < argc; argCounter++)
        {
            p2(filename, argv[argCounter]); // Run fucntion 2 with substring
        }
    }
    else
    {
        // File Name
        filename = argv[1];

        // No flag | Get the substrings
        for (argCounter = 2; argCounter < argc; argCounter++)
        {
            p1(filename, argv[argCounter]); // Run function 1 with substring
        }
    }

    return 0;
}

// Function1 -- Buffer is used
int p1(char *filename, char *substring)
{

    // Opens .txt file
    FILE *file = fopen(filename, "r");

    // Error code if files fails to open
    if (file == NULL)
    {
        perror("./p05"); // Error Message
        return (-1);
    }

    //printf("\n\nSubstrin is :%s\n\n", substring); // Debugging

    // Variables
    char buf[1000];
    int counter = 0;  // Character counter
    int ocounter = 0; // Official Counter
    int k = 0;        // Substring position
    int subSize = 0;  // subString Size

    // Get substring size
    for (int j = 0; substring[j] != '\0'; ++j)
    {
        subSize += 1; // subSize + 1
    }

    // Get buffer
    while (fgets(buf, sizeof(buf), file) != 0)
    {
        // Algorithm
        for (int j = 0; j < sizeof(buf); j++)
        {
            k = 0; // Substring character position

            // Counter for number of times letters are the same
            counter = 0;

            // Check each buffer character to match with substring
            while (buf[j] == substring[k])
            {
                counter++; // Counter + 1
                j++;       // Buffer Postion + 1
                k++;       // SubString Position + 1
            }
            // Check to see if counter equals subString size
            if (counter == subSize)
            {
                ocounter++;      // Offical Counter + 1
                j = j - subSize; // Go to previous position in buffer
                counter = 0;     // Counter reset
            }
        }
    }

    fclose(file); // Close file

    // Print output
    printf("%d\n", ocounter);

    // Return success
    return 0;
}

// Function 2 -- Read each Character
int p2(char *filename, char *substring)
{
    // Opens .txt file
    FILE *file = fopen(filename, "r");

    // Error code if files fails to open
    if (file == NULL)
    {
        perror("./p05"); // Error Message
        return (-1);
    }
    // Variables
    char ch;
    int counter = 0;
    int ocounter = 0;
    int subSize = 0;

    // Get the substring size | Make the substring lowercase
    for (int j = 0; substring[j] != '\0'; ++j)
    {
        subSize += 1; // Substring Size = Substring Size + 1

        // Make Substring lowercase
        if (substring[j] > 65 && substring[j] <= 90)
        {
            // If the letter is uppercase make lowercase
            substring[j] = substring[j] + 32;
        }
    }

    // Start While loop
    while ((ch = fgetc(file)) != EOF)
    {
        // Make UpperCase characters lowercase
        if (ch > 65 && ch <= 90)
        {
            ch = ch + 32;
        }

        // Check to see characters match
        if (ch == substring[0])
        {
            // Check string
            for (int i = 0; i < subSize; i++)
            {
                // If characters match
                if (ch == substring[i])
                {
                    // Counter goes up
                    counter += 1;

                    // If the letter counter is equal to substring size
                    if (counter == subSize)
                    {
                        ocounter += 1; // Official Counter
                        counter = 0;   // Letter counter
                        i -= 1;        // Move one letter back
                    }
                    // Move one letter foward
                    i++;
                }
                // If substring was not found
                else
                {
                    i = subSize; //
                }
            }
        }
    }

    // Print number of times the substring was found
    printf("%d\n", ocounter);

    // Close the file
    fclose(file);

    // Return to main
    return 0;
}
