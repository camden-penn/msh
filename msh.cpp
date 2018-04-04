#include "msh.h"
int main(){
  std::string prompt= "msh > ";
  std::string directory= get_current_dir_name();
  std::map<std::string,std::string> variables;
  std::string input_line="";
  std::vector<std::string>input_tokens;
  std::vector<process>background_processes;
  if(debug_mode){
    variables["^ShowTokens"]="1";
  }
  if(easter_eggs){
    variables["^shrug"] = "¯\\_(ツ)_/¯";
    variables["^table_flip"] = "(ノಠДಠ)ノ ︵ ┻━━━┻";
  }
  variables["^PATH"] = "/bin:/usr/bin";

  while(!std::cin.eof()){
    check_on_background(background_processes);
    std::cout << prompt;
    std::getline(std::cin,input_line,'\n');
    tokenize(input_line,input_tokens,variables);
    evaluate_cmd(input_tokens,background_processes,variables,prompt,directory);
    std::map<std::string,std::string>::iterator search = variables.find("^ShowTokens");
    if(search!=variables.end()&&search->second=="1"){
      //Show all tokens.
      std::cout << "All Tokens: " << std::endl;
      for(unsigned int i=0;i<input_tokens.size();i++){
        std::cout << input_tokens[i] << std::endl;
      }
    }
  }
  return 0;
}
void tokenize(std::string & input_line,std::vector<std::string>& tokens, std::map<std::string, std::string> & variables){
  tokens.clear();
  bool is_string = false;
  std::string current_token="",current_word_in_string="";
  for(unsigned int i=0;i<input_line.length();i++){
    if(input_line[i]=='"'){
      is_string = !is_string;
      if(!is_string){
        resolve_var(current_word_in_string,variables);
        current_token+=current_word_in_string;
        tokens.push_back(current_token);
        current_token="";
        current_word_in_string="";
      }else{
        resolve_var(current_token,variables);
        if(current_token.length()>0){
          tokens.push_back(current_token);
        }
        current_token = "";
      }
    }else if(input_line[i]=='#'){
      i=input_line.length();
    }else if(is_string){
      if(isspace(input_line[i])){
        resolve_var(current_word_in_string,variables);
        current_token+=current_word_in_string+input_line[i];
        current_word_in_string="";
      }else{
        current_word_in_string+=input_line[i];
      }
    }else if(isspace(input_line[i])) {
      resolve_var(current_token,variables);
      if(current_token.length()>0){
        tokens.push_back(current_token);
      }
      current_token = "";
    }else{
      current_token +=input_line[i];
    }
  }
  //End-of-line cleanup
  resolve_var(current_token,variables);
  if(current_token.length()>0){
    tokens.push_back(current_token);
  }
}
void resolve_var(std::string & token, std::map<std::string,std::string> & variables){
  if(token.length()==0||token[0]!='^'){
    return;
  }
  std::map<std::string,std::string>::iterator search = variables.find(token);
  if(search==variables.end()){
    //Var not found
    std::cerr << "Undefined variable: "<< token << std::endl;
    token="";
  }else{
    token=search->second;
  }
  return;
}
void evaluate_cmd(std::vector<std::string> & tokens,std::vector<process> & background_processes, std::map<std::string,std::string> & variables,std::string &prompt,std::string & directory){
  if(!tokens.empty()){
    if(tokens[0] == "setvar"){
      if(tokens.size()<3){
        std::cerr << "Invalid command: missing argument." << std::endl;
      }else{
        variables["^"+tokens[1]] = tokens[2];
      }
    }else if(tokens[0] == "setprompt"){
      if(tokens.size()<2){
        std::cerr << "Invalid command: missing argument." << std::endl;
      }else{
        prompt = tokens[1];
      }
    }else if(tokens[0] == "setdir"){
      if(tokens.size()<2){
        std::cerr << "Invalid command: missing argument." << std::endl;
      }else{
        if(chdir(tokens[1].c_str())==0){
          directory=get_current_dir_name();
        }else{
          perror("Invalid command");
        }
      }
    }else if(tokens[0] == "showprocs"){
      if(background_processes.size()>0){
        std::cout << "Background Processes:" << std::endl;
        for(unsigned int i=0;i<background_processes.size();i++){
          std::cout << to_string(background_processes[i])<< std::endl;
        }
      }else{
        std::cout << "There are no background processes." << std::endl;
      }
    }else if(tokens[0] == "done"){
      if(tokens.size()==1){
        exit(0);
      }else{
        //Get exit status from second token.
        int status;
        try{
          status=std::stoi(tokens[1]);
        }catch(...){
          exit(0);
        }
        if(status < 0){
          exit(0);
        }
        exit(status);
      }
    }else if(tokens[0] == "run"){
      if(tokens.size()>=2){
        pid_t pid = fork();
        if(pid==-1){
          //Error state.
          perror("Error when running command");
        }else if(pid==0){
          //Child process.
          transmute_to(tokens,variables["^PATH"]);
        }else{
          //Parent process.
          //Wait for child to finish.
          int* stats = NULL;
          waitpid(pid,stats,0);
        }
      }else{
        //run [nothing here]? Haha no.
        std::cerr << "Invalid command: missing argument." << std::endl;
      }
    }else if(tokens[0] == "fly"){
      if(tokens.size()>=2){
        pid_t pid = fork();
        if(pid==-1){
          //Error state.
          perror("Error when running command");
        }else if(pid==0){
          //Child process.
          transmute_to(tokens,variables["^PATH"]);
        }else{
          //Parent process.
          //Add child to background processes.
          process new_process;
          new_process.id=pid;
          new_process.name = tokens[1];
          background_processes.push_back(new_process);
        }
      }else{
        //fly [nothing here]? That ain't gonna fly.
        std::cerr << "Invalid command: missing argument." << std::endl;
      }
    }else if(tokens[0] == "tovar"){
      if(tokens.size()>=3){
        int pipefd[2];
        if(pipe(pipefd)==-1){
          perror("Error when running command"); 
        }else{
          pid_t pid = fork();
          if(pid==-1){
            //Error state.
            perror("Error when running command");
          }else if(pid==0){
            //Child process.
            close(pipefd[0]);
            dup2(pipefd[1],1); //redirect std output to pipe
            tokens.erase(tokens.begin()+1);
            transmute_to(tokens,variables["^PATH"]);
          }else{
            //Parent process.
            close(pipefd[1]);
            //When child's finished, absorb its output then resume business as usual.
            int* stats = NULL;
            waitpid(pid,stats,0);
            std::string temp;
            char buf;
            while(read(pipefd[0],&buf,1)>0){
              temp+=buf;
            }
            close(pipefd[0]);
            variables["^"+tokens[1]] = temp;//entire contents of pipefd[0]
          }
        }
      }else{
        //Something's missing.
        std::cerr << "Invalid command: missing argument." << std::endl;
      }

    }else{
      std::cerr << "Invalid command '" << tokens[0] << "'." << std::endl;
    }
  }
}
void check_on_background(std::vector<process> & background_processes){
  int*stats = NULL;
  int val_returned;
  for(std::vector<process>::iterator index = background_processes.begin();index!=background_processes.end();index++){
    val_returned=waitpid((*index).id,stats,WNOHANG);
    if(val_returned<0){
      perror(NULL);
    }else if(val_returned>0){
      std::cout << "Completed: " << (*index).name << std::endl;
      index = background_processes.erase(index) - 1;
    }
  }
}
void transmute_to(std::vector<std::string> & tokens,std::string & PATH){
  //Turns a child process into the command given in tokens.
  char**args = new char*[tokens.size()];
  for(unsigned int i=1;i<tokens.size();i++){
    args[i-1] = new char[tokens[i].length()+1];
    std::strcpy(args[i-1],tokens[i].c_str());
  }
  args[tokens.size()-1]=(char*)0;
  char path[5]="PATH";
  char*path_val = new char[PATH.length()+1];
  std::strcpy(path_val,PATH.c_str());
  setenv(path,path_val,1);
  execvpe(tokens[1].c_str(),args,environ);
  perror("Failed to run");
  exit(1);
}
