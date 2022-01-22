#include <iostream>
#include <vector>
#include <list>
#include <chrono>

#include "data.h"
#include "hungarian.h"

Data* data;
double** cost_matrix;
int mode;

typedef struct
{
    std::vector<std::pair<int, int>> prohibited_edges;
    std::vector<std::vector<int>> subtours;
    double lower_bound;
} Node;

void depth_first_search();

int main(int argc, char** argv)
{
    data = new Data(argc, argv[1]);
	data->readData();

    mode = HUNGARIAN_MODE_MINIMIZE_COST;

    if(std::stoi(argv[2]) == 1)
    {
        depth_first_search();
    }

    delete data;

	return 0;
}

void print_node(Node* node, bool lower_bound, bool subtours, bool prohibited_edges)
{
    if(lower_bound)
    {
        printf("Assignment: %.2lf\n", (*node).lower_bound);
    }
    if(subtours)
    {
        printf("Subtours: {");
        for(int i = 0; i < (*node).subtours.size(); i++)
        {
            printf("{");
            for(int j = 0; j < (*node).subtours[i].size(); j++)
            {
                printf("%d,", (*node).subtours[i][j]);
            }
            printf("}, ");
        }
        printf("}\n");
    }
    if(prohibited_edges)
    {
        printf("Prohibited Edges: ");
        for(int i = 0; i < (*node).prohibited_edges.size(); i++)
        {
            printf("(%d, %d), ", (*node).prohibited_edges[i].first, (*node).prohibited_edges[i].second);
        }
        printf("\n");
    }
}

void define_cost_matrix(std::vector<std::pair<int, int>>& prohibited_edges)
{
    cost_matrix = new double*[data->getDimension()];
	for(int i = 0; i < data->getDimension(); i++)
    {
		cost_matrix[i] = new double[data->getDimension()];
		for(int j = 0; j < data->getDimension(); j++)
        {
			cost_matrix[i][j] = data->getDistance(i,j);
		}
	}
    for(int i = 0; i < prohibited_edges.size(); i++)
    {
        cost_matrix[(prohibited_edges[i].first)-1][(prohibited_edges[i].second)-1] = INFINITE;
    }
}

void construct_subtours(int*** assignment, std::vector<std::vector<int>>& subtours)
{
    std::vector<bool> vertex_not_in_subtour;
    for(int i = 0; i < data->getDimension(); i++)
    {
        vertex_not_in_subtour.push_back(true);
    }
    for(int i = 0; i < data->getDimension(); i++)
    {
        if(vertex_not_in_subtour[i])
        {
            int j = i;
            std::vector<int> subtour;
            while(true)
            {
                for(int k = 0; k < data->getDimension(); k++)
                {
                    if(vertex_not_in_subtour[k])
                    {
                        if((*assignment)[j][k])
                        {
                            vertex_not_in_subtour[k] = false;
                            subtour.push_back(k+1);
                            j = k;
                            break;
                        }
                    }
                }
                if(j == i)
                    break;
            }
            subtour.insert(subtour.begin(), i+1);
            subtours.push_back(subtour);
        }
    }
}

void free_memory(hungarian_problem_t* hp)
{
    hungarian_free(hp);
	for (int i = 0; i < data->getDimension(); i++)
    {
        delete [] cost_matrix[i];
    }
    delete [] cost_matrix;
}

void solve(Node& node)
{
    define_cost_matrix(node.prohibited_edges);
    hungarian_problem_t hp;
    hungarian_init(&hp, cost_matrix, data->getDimension(), data->getDimension(), mode);
	node.lower_bound = hungarian_solve(&hp);
    construct_subtours(&(hp.assignment), node.subtours);
    free_memory(&hp);
}

bool has_better_bound(double& node_bound, double& best_bound)
{
    return node_bound < best_bound;
}

bool has_invalid_subtour(std::vector<std::vector<int>>& subtours, std::vector<std::pair<int, int>>& prohibited_edges, double node_bound)
{
    //both methods bellow work
    // for(int i = 0; i < subtours.size(); i++)
    // {
    //     if(subtours[i].size() <= 2)
    //     {
    //         return true;
    //     }
    //     for(int j = 0; j < subtours[i].size()-1; j++)
    //     {
    //         for(int k = 0; k < prohibited_edges.size(); k++)
    //         {
    //             if((subtours[i][j] == prohibited_edges[k].first) && 
    //                (subtours[i][j+1] == prohibited_edges[k].second))
    //             {
    //                 return true;
    //             }
    //         }
    //     }
    // }
    // return false;
    return node_bound >= INFINITE;    
}

bool has_one_subtour(std::vector<std::vector<int>>& subtours)
{
    return subtours.size() == 1;
}

void update_best_node(Node& current_node, Node** best_node)
{
    delete (*best_node);
    (*best_node) = new Node(
        {
            current_node.prohibited_edges,
            current_node.subtours,
            current_node.lower_bound
        }
    );
}

void branch(Node& current_node, std::list<Node>& tree)
{
    int choosen_subtour = 0;
    int smaller_size = current_node.subtours[0].size();
    for(int i = 1; i < current_node.subtours.size(); i++)
    {
        if(current_node.subtours[i].size() < smaller_size)
        {
            choosen_subtour = i;
            smaller_size = current_node.subtours[i].size();
        }
    }
    // for(int i = smaller_size-1; i > 0; i--)
    // {
    //     std::vector<std::pair<int, int>> prohibited_edges = current_node.prohibited_edges;
    //     prohibited_edges.push_back(
    //         make_pair(
    //             current_node.subtours[choosen_subtour][i-1], 
    //             current_node.subtours[choosen_subtour][i]
    //         )
    //     );
    //     prohibited_edges.push_back(
    //         make_pair(
    //             current_node.subtours[choosen_subtour][i], 
    //             current_node.subtours[choosen_subtour][i-1]
    //         )
    //     );
    //     tree.insert(++tree.begin(), {prohibited_edges});
    //     if(smaller_size == 3)
    //         return;
    // }
    for(int i = 0; i < smaller_size-1; i++)
    {
        std::vector<std::pair<int, int>> prohibited_edges = current_node.prohibited_edges;
        prohibited_edges.push_back(
            make_pair(
                current_node.subtours[choosen_subtour][i], 
                current_node.subtours[choosen_subtour][i+1]
            )
        );
        tree.insert(++tree.begin(), {prohibited_edges});
    }
}

void prune(std::list<Node>& tree)
{
    tree.pop_front();
}

void depth_first_search()
{
    Node root;
    std::list<Node> tree;
    Node* best_node = new Node();
    best_node->lower_bound = INFINITE;

    tree.push_back(root);

    auto timer_start = chrono::system_clock::now();
    while(!tree.empty())
    {
        Node& current_node = tree.front();
        solve(current_node);
        if(has_better_bound(current_node.lower_bound, best_node->lower_bound))
        {
            if(!has_invalid_subtour(current_node.subtours, current_node.prohibited_edges, current_node.lower_bound))
            {
                if(has_one_subtour(current_node.subtours))
                {
                    update_best_node(current_node, &best_node);
                }
                else
                {
                    branch(current_node, tree);
                }
            }
        }
        prune(tree);
    }
    auto timer_end = chrono::system_clock::now();
    chrono::duration<double> duration = timer_end - timer_start;
    
    printf("BEST NODE:\n");
    if(best_node->lower_bound < INFINITE)
    {
        print_node(best_node, true, true, true);
    }
    printf("TIME: %.2lfs\n", duration.count());
}