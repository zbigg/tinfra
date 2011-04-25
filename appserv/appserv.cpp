struct Command {
    typedef std::string CommandName;
    typedef std::vector<std::string> CommandArgs;
    
    CommandName command;
    CommandArgs arguments;
};

struct Task {
    Command     command;
    Environment env;
    
    Channel     input;
    Channel     output;
};

/**
  Worker is a separate thread, possibly process.

  Manager manages workers lifetime
    Manager starts worker
    Worker after start becomes IDLE

  Worker 
    has 3 states
        IDLE
        WORKING
        WRITING
    when does "functional", not mutating work is WORKING
    when does some commit work becomes WRITING
    can be forcefully terminated safely only when in IDLE | WORKING state
    musn't be forcifullly terminated when in WRITING phase

        NOTE above results in a need for synchronous "manager.allowWrite(self)" request
        that enables worker to commit work
        this increases  communication cost (CONTROL bandwidth and overall mutating jobs latency)
        
        summarizing: each state change must be ATOMICALLY confirimed between worker&manager
        
  Worker 
    has its input queue (may be shared with others!)
    has its complete tasks storage
    have its
        task storage
        
        intermediate & temporary storage (is a current working folder of worker process)
    
    it polls (or is signalled) input queue for tasks
    when accepts job, atomically moves task from IQ to task storage
    when finishes job, moves task to completed tasks storage
    when aborted, leave all files from task storage in its folder
    when creating intermediate, input files in task folder, always does it atomically
        (no incomplete file is found)
    
sends a Tnotification
    
  Manager kills worker 
    Manager should stop only IDLE worker
    Manager should send termination message to worker

class Worker {
    // ptr to process data
    enum State {
        IDLE,
        BUSY
    };
    
};

int main(int argc, char** argv)
{
    
}

