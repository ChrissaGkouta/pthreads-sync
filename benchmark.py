import subprocess
import statistics

# Παράμετροι πειραμάτων
thread_counts = [1, 2, 4, 8]  
iterations = 1000000          # 1 εκατομμύριο επαναλήψεις
modes = ["mutex", "rwlock", "atomic"]
runs_per_experiment = 4

print("Mode,Threads,Iterations,AvgTime")

# Compilation
subprocess.run(["make"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

for mode in modes:
    for t in thread_counts:
        times = []
        for _ in range(runs_per_experiment):
            # Εκτέλεση του C προγράμματος
            result = subprocess.run(
                ["./pthreads_sync", str(t), str(iterations), mode],
                capture_output=True, text=True
            )
            # Ανάγνωση της εξόδου (CSV format από την C)
            output_parts = result.stdout.strip().split(',')
            if len(output_parts) >= 5:
                exec_time = float(output_parts[4])
                times.append(exec_time)
        
        avg_time = statistics.mean(times)
        print(f"{mode},{t},{iterations},{avg_time:.6f}")