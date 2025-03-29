enum Signal_split {
	NONE,
	START,
	SPLIT,
	PAUSE
	//add your signals here
};

inline const std::string To_String(Signal_split sig)
{
    switch (sig)
    {
        case START:
		   return "START";

        case SPLIT:
		   return "SPLIT";

        case PAUSE:
			return "PAUSE";
			
        default:
			return "NONE";
    }
}