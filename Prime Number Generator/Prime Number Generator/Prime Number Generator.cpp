//2.5.26-2.9.26
//This is a tool to automatically list all prime numbers in sequential order, and save them to an outfile if the user chooses to.

//Preprocessor Libraries
#include <iostream> //For input/output
#include <fstream> //For outfile creation
#include <vector> //For vectors to store prime numbers
#include <mutex> //For mutex vector protection
#include <climits> //For the INT_MAX constant
#include <thread> //For running slo-mo tests and mid-generation pausing
#include <atomic> //For mid-generation pausing
#include <chrono> //For running slo-mo tests

using namespace std; //Declare standard namespace

const string INDENT = "\t\t\t"; //Size of indent in wait=3 tabs

atomic<bool> pauseRequested(false); //Universal pause flag
atomic<bool> saveRequested(false); //Universal save flag
atomic<bool> paused(false); //Universal flag to check if currently paused

mutex primesMutex; //Universal mutex flag to prevent modification to either vector while saving to an outfile

void listenForKeyInput(); //Function to detect presses of the enter key while the generator is running and presses of enter or s (to save) while paused
void pauseGeneration(const vector<int>& Primes, const vector<long long>& LargePrimes); //Works with listenForKeyInput to stop prime generation when the enter key is pressed, allowing all generated primes to be saved as well
void savePrimes(const vector<int>& Primes, const vector<long long>& LargePrimes); //Function to save all primes generated so far (in the "Primes" vector) to a text document called "Primes.txt"
void wait(); //Pause alternative
void clearCin(); //Clears cin field for wait function or wipe cin memory

int main() {
	char ch;
	cout << "Henry Kredatus's C++ Prime Number Generator\n\n"; //Project title
	cout << "Press ENTER to begin generating primes. Press ENTER again at anytime to pause generation,\nwhich also gives you the option to save all currently generated primes to a text file: "; //Prompt to press enter to begin

	while (true) {
		cin.get(ch); //Record the entered key
		clearCin(); //Get rid of any trailing characters

		if (ch == '\n') { //Pressing JUST enter begins the prime number generation
			break;
		}

		else {  //If the entered key wasn't s/S or enter, prompts for another key input
			cout << "\nPressed key was not enter (JUST enter). Try again: ";
		}
	}

	cout << endl; //Drop a line before resuming generation

	thread inputThread(listenForKeyInput); //Begin permanently attempting to detect presses of the enter key while the generator runs and presses of s OR enter while paused
	inputThread.detach();

	int potentialPrime = 0; //Runs through every integer starting with zero (up to just below the integer limit for ints)
	long long potentialLargePrime = static_cast<long long>(INT_MAX); //Used to test potential prime numbers starting with the integer limit for ints (starts right where the int checks end)

	vector<int>Primes; //Create a vector to store every generated prime below the integer limit for ints that can then be used to save them to a file
	vector<long long>LargePrimes; //Create a vector to store every prime starting with the integer limit for ints, which can also be saved to a file

	while (potentialPrime < INT_MAX) { //Loop through all integers (below the integer limit for ints)

		bool neitherCheck = false; //Start by assuming the integer IS prime or composite
		bool compositeCheck = false; //Start by assuming the integer IS prime specifically

		if (potentialPrime > 1) { //Primes must be at least two by definition (first check)

			//Former attempt to save time, the thought being excluding multiples of two greater than two would be faster as no such numbers can be prime (second check)
			//Didn't work due to factoring in the extra if/else check being slower than running the nested for loop
			//if ((potentialPrime % 2 != 0) || (potentialPrime == 2)) {
				
				int potentialFactor = 2; //Runs through every possible integer factor for the number whose prime status is being tested starting with two, up to the potentialPrime's square root, or the first factor it hits
				
				while (potentialFactor * potentialFactor <= potentialPrime) { //Loop through all integers between two and the square root of the number whose prime status is being tested (as any number with an integer factor greater than one and less than or equal to its square root is prime) to see if at least one is indeed a factor

					//Final check
					if (potentialPrime % potentialFactor == 0) { //If the remainder from dividing the potential prime integer by a potential integer factor was zero, then the number divided evenly, the divisor was indeed a factor, and the dividend is composite (ending the loop for that integer immediately)
						compositeCheck = true; //Set composite status to true
						break;
					}

					potentialFactor++; //Move on to the next potential factor (integer) if the previous integer wasn't a factor
				}

			//} //End former "if multiple of two that's greater than two" check

			//else {
			//	compositeCheck = true; //If the number was a multiple of two other than two, that means it's composite
			//}

		} //End "is prime or composite" check

		else {
			neitherCheck = true; //If the tested integer above was zero or one, set the flag for being neither prime nor composite to true
		}

		if (compositeCheck == false && neitherCheck == false) {
			{
				lock_guard<mutex> lock(primesMutex); //Lock when saving a prime to the vector
				Primes.push_back(potentialPrime); //If the number wasn't proven to be zero, one, or composite, then it's prime and stored in the Primes vector
			}
			cout << potentialPrime << endl; //If the number wasn't proven to be zero, one, or composite, then it's prime and is printed in the list

			//This line should have been faster than the above one in theory without the buffer flush, but proved slightly slower in practice
			//cout << potentialPrime << '\n';
		}

		//this_thread::sleep_for(chrono::seconds(1)); //Delay used to put generator in slo-mo during debugging

		if (pauseRequested) { //If the pauseRequested flag is triggered by an enter press, turns the flag off and and runs the pauseGeneration function (enabling saving as well)
			pauseRequested = false; 
			paused = true;
			pauseGeneration(Primes, LargePrimes);
		}

		potentialPrime++; //Move on the next integer

	} //End big integer while loop

	//The generator uses two loops-one for "small" integers, and one for any integers at or above the integer limit for ints
	//This allows the generator to perform at max speed while running through smaller integers without it being forced to stop (and roll over) when the int integer limit is reached
	//The loop switch took a little over six hours of the generator constantly running to occur during my testing
	//The generator will briefly stall when the loop switch does happen

	while (true) { //Loop through all integers up to the integer limit for long longs (effectively forever), starting right where the previous loop left off

			bool largePrimeCompositeCheck = false; //Start by assuming the long long integer IS prime specifically
			long long potentialLargePrimeFactor = 2; //Runs through every possible integer factor for the number whose prime status is being tested starting with two, up to the potentialLargePrime's square root, or the first factor it hits

			while (potentialLargePrimeFactor <= potentialLargePrime / potentialLargePrimeFactor) { //Loop through all integers between two and the square root of the number whose prime status is being tested (as any number with an integer factor greater than one and less than or equal to its square root is prime) to see if at least one is indeed a factor

				//Check if the long long integer is prime or composite
				if (potentialLargePrime % potentialLargePrimeFactor == 0) { //If the remainder from dividing the potential prime long long integer by a potential long long integer factor was zero, then the number divided evenly, the divisor was indeed a factor, and the dividend is composite (ending the loop for that long long integer immediately)
					largePrimeCompositeCheck = true; //Set composite status to true
					break;
				}

				potentialLargePrimeFactor++; //Move on to the next potential factor (long long integer) if the previous long long integer wasn't a factor
			}

		if (largePrimeCompositeCheck == false) { //Doesn't need to perform a neither check as the long long loop starts at the integer limit for ints
			{
				lock_guard<mutex> lock(primesMutex); //Lock when saving a prime to the vector
				LargePrimes.push_back(potentialLargePrime); //If the number wasn't proven to be composite, then it's prime and stored in the LargePrimes vector
			}
			cout << potentialLargePrime << endl; //If the number wasn't proven to be composite, then it's prime and is printed in the list

			//This line should have been faster than the above one in theory without the buffer flush, but proved slightly slower in practice
			//cout << potentialPrime << '\n';
		}

		//this_thread::sleep_for(chrono::seconds(1)); //Delay used to put generator in slo-mo during debugging

		if (pauseRequested) { //If the pauseRequested flag is triggered by an enter press, turns the flag off and and runs the pauseGeneration function (enabling saving as well)
			pauseRequested = false;
			paused = true;
			pauseGeneration(Primes, LargePrimes);
		}

		potentialLargePrime++; //Move on the next long long integer

	} //End big long long while loop

	wait();
	return 0;
}








void listenForKeyInput() { //Function to detect presses of the enter key while the generator is running and presses of enter or s (to save) while paused
	while (true) { //Constantly running for one reason or the other
		char ch = cin.get(); //Permanently attempts to detect presses of the enter key while the generator runs and presses of s OR enter while paused

		if (ch == '\n') {
			if (!paused) {
				pauseRequested = true; //Pauses if enter is pressed and generator is currently running
			}

			else if (!saveRequested)
			{
				paused = false; //Resumes if enter is pressed again, currently paused, and NOT currently saving
			}
		}

		if ((ch == 's' || ch == 'S') && paused) { //If currently paused, pressing s sets saveRequested to true and runs the save function
			saveRequested = true;
		}
	}
}








void pauseGeneration(const vector<int>& Primes, const vector<long long>& LargePrimes) { //Works with listenForKeyInput to stop prime generation when the enter key is pressed, allowing all generated primes to be saved as well
	cout << "Prime generation paused.\n";
	cout << "Press ENTER to resume, or press S then ENTER to save: ";

	while (paused) { //The REAL pause happens here, as this loop (and therefore, function) won't end and allow the generator loop in main to continue until listenForKeyInput detects that enter is pressed again
		if (saveRequested) { //Runs the save function if listenForKeyInput detects a save request
			saveRequested = false;
			savePrimes(Primes, LargePrimes);
		}

		this_thread::sleep_for(chrono::milliseconds(10)); //Helps conserve CPU usage
	}

	cout << endl; //Drop a line before resuming generation
}








void savePrimes(const vector<int>& Primes, const vector<long long>& LargePrimes) { //Function to save all primes generated so far (in the "Primes" vector) to a text document called "Primes.txt"
	lock_guard<mutex> lock(primesMutex); //Lock during the entire save to file process
	cout << "\nSaving all generated primes to \"Primes.txt\"-This might take a bit-Check the source files afterward!\n\n";
	cout << "Now saving...\n\n";

	ofstream outfile("Primes.txt"); //Create outfile variable

	if (outfile.is_open()) { //Makes sure outfile was created and opened successfully

		outfile << Primes.size() + LargePrimes.size() << " primes have been generated so far.\n"; //Print the current number of generated and saved primes

		if (LargePrimes.size() == 0) //If the generator hasn't yet reached the integer limits for ints, then the larget current prime is in the Primes vector
			outfile << "Largest prime generated so far: " << Primes.back() << endl; //Print the largest currently-generated prime.

		if (LargePrimes.size() > 0) //If the generator has reached the integer limits for ints, then the larget current prime is in the LargePrimes vector
			outfile << "Largest prime generated so far: " << LargePrimes.back() << endl; //Print the largest currently-generated prime.

		outfile << "All primes generated so far:\n"; //List heading

		for (int i = 0; i < Primes.size();i++) { //Loop through the entire Primes vector (all primes below the integer limit for ints that have been generated so far)
			outfile << Primes.at(i) << endl; //Print every generated prime on its on line
		}

		if (LargePrimes.size() > 0) { //If the generator has reached the integer limits for ints, then all stored primes above it are looped through as well
			for (int i = 0; i < LargePrimes.size();i++) { //Loop through the entire LargePrimes vector (all primes at or above the integer limit for ints that have been generated so far)
				outfile << LargePrimes.at(i) << endl; //Print every generated prime on its on line
			}
		}

		cout << "Primes saved successfully-Press the ENTER key to resume generation. "; //Confirm successful save
	}

	else { //Error message if outfile fails to open, which should never happen
		cout << "Cannot open file!\n";
		wait();
		exit(0);
	}
}








void wait() //Pause alternative
{
	clearCin();
	char ch;
	cout << endl << INDENT << "Press the Enter key to continue ... ";
	cin.get(ch);
}








void clearCin()
{
	//The following if-statement checks to see how many characters are in cin's buffer
	//If the buffer has characters in it, the ignore method gets rid of them.
	//If cin is in the fail state, clear puts it back to the ready state.
	//If cin is not already in the fail state, it still doesn't hurt to call the clear function.
	if (cin.rdbuf()->in_avail() > 0) //If the buffer is empty skip clear and ignore
	{
		cin.clear();
		cin.ignore(numeric_limits<streamsize>::max(), '\n'); //Clear the input buffer
	}
}