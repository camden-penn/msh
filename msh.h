#include <map>
#include <vector>
#include <iostream>
#include <string>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "process.h"
#define debug_mode false
#define easter_eggs true
/*
  tokenize()
  ----------
  Inputs:
    input_line: a std::string to be converted to tokens.
    tokens: a std::vector to store the resultant tokens.
    variables: A std::map of variables, used for variable resolution.

  End conditions:
    tokens contains a tokenized representation of input_line.
    All variables in input_line have been resolved in tokens.  
*/
void tokenize(std::string & input_line,std::vector<std::string>& tokens, std::map<std::string, std::string> & variables);

/*
  resolve_var()
  ------------
  Inputs:
    token: A token to resolve (iff it is a variable).
    variables: The map of variables.

  End conditions:
    If token was not a variable, token is unchanged.
    If token was a variable, resolution was attempted.
      If resolution was successful, token is now the resolved value.
      Otherwise, token is now "" and an error message has been printed.
*/
void resolve_var(std::string & token, std::map<std::string,std::string> & variables);

/*
  evaluate_cmd()
  --------------
  Inputs:
    tokens: The command to run as a vector of tokens.
    background_processes: A list of currently active background processes.
    variables: The map of defined variables.
    prompt: The current prompt.
    directory: The current working directory.

  Attempts to parse the command provided in tokens and take appropriate action.
    Possible actions include:
      setvar: Adds [or updates] a variable in variables.
      setdir: Changes the working directory.
      setprompt: Changes the command prompt text.
      showprocs: Lists background processes.
      done: exits.

      run, fly, tovar:
        Creates a child process to do its bidding.
        run: in foreground.
        fly: in background.
        tovar: in foreground, but pipes result to a variable. 
*/
void evaluate_cmd(std::vector<std::string> & tokens,std::vector<process> & background_processes, std::map<std::string,std::string> & variables,std::string &prompt,std::string & directory);

/*
  check_on_background()
  ---------------------

  Checks on the progress of each child in background_processes.
    If child is done:
      Prints message saying child is complete.
      Reaps child.
      Removes child from list of background processes.
*/
void check_on_background(std::vector<process> & background_processes);

/*
  transmute_to()
  --------------  
  Only called by child processes.
  Attempts to transmute self into the command described by tokens.
  Uses execvpe() to easily search PATH, if necessary.
*/
void transmute_to(std::vector<std::string> & tokens,std::string & PATH);
